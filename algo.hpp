#pragma once
#include <cassert>
#include <chrono>
#include <cmath>
#include <iostream>
#include <random>
#include "util.hpp"

namespace algo {

template<class GameTraits, class RNG = std::mt19937>
class Random {
 public:
  using Board = typename GameTraits::Board;
  using Move = typename GameTraits::Move;
  using History = typename GameTraits::History;

  Random(RNG& rng) : rng_(rng) {}

  const char* GetName() const { return "Random"; }

  Move GetNextMove(const Board& board, const History& history) {
    const auto legal_moves = board.GetLegalMoves();
    assert(!legal_moves.empty());
    std::uniform_int_distribution<> dis(0, legal_moves.size() - 1);
    return legal_moves[dis(rng_)];
  }

 private:
  RNG& rng_;
};

template<class GameTraits, bool Debug = false, class RNG = std::mt19937>
class GenericMCTS {
 public:
  using Board = typename GameTraits::Board;
  using Move = typename GameTraits::Move;
  using Player = typename GameTraits::Player;
  using History = typename GameTraits::History;

  GenericMCTS(RNG& rng, const std::chrono::seconds thinking_time)
      : rng_(rng),
        bias_(1.4),
        thinking_time_(thinking_time) {
  }

  void SetBias(const double b) { bias_ = b; }

  const char* GetName() const { return "GenericMCTS"; }

  // for non-root nodes, (parent->board, this->move) -> this->board
  struct Node {
    double value;        // num_wins / num_visited
    double num_wins;     // number of wins from the player who made the move
    size_t num_visited;  // number of simulations from this and all descendant nodes
    Node* parent;
    Node* child;
    Node* sibling;
    Board board;   // for root node: the initial game state; otherwise the resulting state
    Move move;     // for non-root nodes: the taken move from parent's state

    // the player who played the taken move
    Player GetPlayer() const {
      return parent ? parent->board.current_player() : board.current_player();
    }
  };
  using Nodes = util::FixedBulk<Node, 10000>;

  Move GetNextMove(const Board& board, const History& history) {
    const auto start_time = std::chrono::high_resolution_clock::now();
    Move m;
    size_t iter = 0;
    {
      Node& root = nodes_.Create();
      root.value = 0,
      root.num_wins = 0;
      root.num_visited = 0;
      root.parent = nullptr;
      root.child = nullptr;
      root.sibling = nullptr;
      root.board = board;
      root.move = GameTraits::GetIllegalMove();
      do {
        for (size_t j = 0; j < 100; ++j) {
          ++iter;
          Node& leaf = Select(root);
          Node& child = leaf.board.IsFinished() ? leaf : Expand(leaf);
          SimulateAndUpdate(child);
        }
      } while (std::chrono::high_resolution_clock::now() - start_time < thinking_time_);
      assert(root.child);
      const Node* child = root.child;
      const Node* best = child;
      double best_value = best->value;
      while (child->sibling) {
        child = child->sibling;
        if (child->value > best_value) {
          best = child;
          best_value = best->value;
        }
      }
      if (Debug) {
        child = root.child;
        while (child) {
          std::cout << "[GenericMCTS] Move: ";
          GameTraits::PrintMove(std::cout, child->move);
          std::cout << ", v_i = " << child->value
                    << ", n_i = " << child->num_visited << std::endl;
          child = child->sibling;
        }
        std::cout << "[GenericMCTS] Choosen move: ";
        GameTraits::PrintMove(std::cout, best->move);
        std::cout << ", v_i = " << best->value
                  << ", n_i = " << best->num_visited << std::endl;
      }
      m = best->move;
    }
    const size_t num_nodes = nodes_.size();
    nodes_.clear();
    const auto end_time = std::chrono::high_resolution_clock::now();
    if (Debug) {
      const auto t = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count() / 1000000.0;
      std::cout << "[GenericMCTS] Iterated " << iter << " times for "
                << t << " sec ("
                << iter / t << " iter/s)" << std::endl
                << "[GenericMCTS] " << num_nodes << " nodes created ("
                << sizeof(Node) << " bytes/node, ~"
                << num_nodes * sizeof(Node) << " bytes)" << std::endl;
    }
    return m;
  }

 private:
  Node& Select(Node& root) {
    const auto c = bias_;
    Node* leaf = &root;
    while (leaf->child) {
      const double logn = std::log(util::at_least_1(leaf->num_visited));
      auto q_value = [logn, c] (const Node* n) { 
        return n->value + c * std::sqrt(logn / util::at_least_1(n->num_visited));
      };
      leaf = leaf->child;
      double ucb = q_value(leaf);
      Node* child = leaf;
      while (child->sibling) {
        child = child->sibling;
        const double v = q_value(child);
        if (v > ucb) {
          ucb = v;
          leaf = child;
        }
      }
    }
    return *leaf;
  }

  Node& Expand(Node& node) {
    const auto i = nodes_.size();
    for (const auto m : node.board.GetLegalMoves()) {
      Node& child = nodes_.Create();
      child.value = 0;
      child.num_wins = 0;
      child.num_visited = 0;
      child.parent = &node;
      child.child = nullptr;
      child.sibling = nullptr;
      child.board = node.board;
      child.move = m;
      child.board.Next(m);
      if (node.child) child.sibling = node.child;
      node.child = &child;
    }
    std::uniform_int_distribution<> dis(i, nodes_.size() - 1);
    return nodes_[dis(rng_)];
  }

  void SimulateAndUpdate(Node& node) {
    Board board = node.board;
    while (!board.IsFinished()) {
      const auto m = GetRandomMove(board);
      board.Next(m);
    }
    const auto winner = board.winner();
    const bool draw = board.IsDraw();
    Node* p = &node;
    for (;;) {
      ++p->num_visited;
      if (p->GetPlayer() == winner) {
        p->num_wins += 1;
      } else if (draw) {
        //p->num_wins += .5;
      }
      p->value = p->num_wins / p->num_visited;
      if (!p->parent) break;
      p = p->parent;
    }
  }

  Move GetRandomMove(const Board& board) {
    const auto legal_moves = board.GetLegalMoves();
    assert(!legal_moves.empty());
    std::uniform_int_distribution<> dis(0, legal_moves.size() - 1);
    return legal_moves[dis(rng_)];
  }

  RNG& rng_;
  double bias_;
  std::chrono::seconds thinking_time_;
  Nodes nodes_;
};

}  // namespace algo
