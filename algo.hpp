#pragma once
#include <cassert>
#include <chrono>
#include <cmath>
#include <iostream>
#include <random>
#include "util.hpp"

namespace algo {

template<class GameTraits, bool Debug = false>
class GenericMCTS {
 public:
  using Board = typename GameTraits::Board;
  using Move = typename GameTraits::Move;
  using Player = typename GameTraits::Player;
  using History = typename GameTraits::History;

  GenericMCTS(std::chrono::seconds thinking_time)
      : rd_(),
        gen_(rd_()),
        bias_(1.4),
        thinking_time_(thinking_time) {
  }

  void SetBias(double b) { bias_ = b; }

  const char* GetName() const { return "GenericMCTS"; }

  struct Node {
    Board board;
    Player player;
    Move move;
    double value;
    double num_wins;
    size_t num_visited;
    Node* parent;
    Node* left_child;
    Node* right_sibling;
  };
  using Nodes = util::FixedBulk<Node, 10000>;

  Move GetNextMove(const Board& board, const History& history) {
    const auto start_time = std::chrono::high_resolution_clock::now();
    const auto me = board.current_player();
    Move m;
    size_t iter = 0;
    {
      Node& root = nodes_.Create();
      root.board = board;
      root.player = me;
      root.move = -1;
      root.value = 0,
      root.num_wins = 0;
      root.num_visited = 0;
      root.parent = nullptr;
      root.left_child = nullptr;
      root.right_sibling = nullptr;
      do {
        for (size_t j = 0; j < 100; ++j) {
          ++iter;
          Node& leaf = Select(root);
          Node& child = leaf.board.IsFinished() ? leaf : Expand(leaf);
          SimulateAndUpdate(child, me);
        }
      } while (std::chrono::high_resolution_clock::now() - start_time < thinking_time_);
      assert(root.left_child);
      const Node* child = root.left_child;
      const Node* best = child;
      double best_value = best->value;
      while (child->right_sibling) {
        child = child->right_sibling;
        if (child->value > best_value) {
          best = child;
          best_value = best->value;
        }
      }
      if (Debug) {
        child = root.left_child;
        while (child) {
          std::cout << "[GenericMCTS] Move: ";
          GameTraits::PrintMove(std::cout, child->move);
          std::cout << ", v_i = " << child->value
                    << ", n_i = " << child->num_visited << std::endl;
          child = child->right_sibling;
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
    while (leaf->left_child) {
      const double logn = std::log(util::at_least_1(leaf->num_visited));
      auto q_value = [logn, c] (const Node* n) { 
        return n->value + c * std::sqrt(logn / util::at_least_1(n->num_visited));
      };
      leaf = leaf->left_child;
      double ucb = q_value(leaf);
      Node* child = leaf;
      while (child->right_sibling) {
        child = child->right_sibling;
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
      child.board = node.board;
      child.player = GameTraits::GetOppositePlayer(node.player);
      child.move = m;
      child.value = 0;
      child.num_wins = 0;
      child.num_visited = 0;
      child.parent = &node;
      child.left_child = nullptr;
      child.right_sibling = nullptr;
      child.board.Next(m);
      if (node.left_child) child.right_sibling = node.left_child;
      node.left_child = &child;
    }
    std::uniform_int_distribution<> dis(i, nodes_.size() - 1);
    return nodes_[dis(gen_)];
  }

  void SimulateAndUpdate(Node& node, const Player me) {
    Board board = node.board;
    while (!board.IsFinished()) {
      const auto m = GetRandomMove(board);
      board.Next(m);
    }
    const bool won = board.winner() == me;
    const bool draw = board.IsDraw();
    Node* p = &node;
    for (;;) {
      ++p->num_visited;
      if (won) {
        p->num_wins += 1;
      } else if (draw) {
        //p->num_wins += .5;
      }
      if (p->player == me) {
        p->value = 1 - p->num_wins / p->num_visited;
      } else {
        p->value = p->num_wins / p->num_visited;
      }
      if (!p->parent) break;
      p = p->parent;
    }
  }

  Move GetRandomMove(const Board& board) {
    const auto legal_moves = board.GetLegalMoves();
    assert(!legal_moves.empty());
    std::uniform_int_distribution<> dis(0, legal_moves.size() - 1);
    return legal_moves[dis(gen_)];
  }

  std::random_device rd_;
  std::mt19937 gen_;
  double bias_;
  std::chrono::seconds thinking_time_;
  Nodes nodes_;
};

}  // namespace algo
