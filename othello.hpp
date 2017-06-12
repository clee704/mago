#pragma once
#include "util.hpp"

namespace othello {

using BoardSize = uint8_t;
using Move = int8_t;
using Player = uint8_t;
using CellValue = uint8_t;  // use only the last 3 bits

enum : CellValue {
  DARK       = 0b001,
  LIGHT      = 0b111,
  NONE_DARK  = 0b010,  // empty cell, dark may place
  NONE_LIGHT = 0b100,  // empty cell, light may place
  NONE_BOTH  = 0b110,  // empty cell, anyone may place
  NONE       = 0b000,
};

inline const char* ToSymbol(const CellValue v) {
  return v == DARK ? TERM_GRAY("@") : v == LIGHT ? "O" : ".";
}

inline constexpr Player GetOppositePlayer(const Player p) {
  return p ^ 0b110;
}

inline constexpr bool IsEmpty(const CellValue v) {
  return (v & 0b001) == 0;
}

inline constexpr CellValue TogglePlaceable(const CellValue v) {
  return v ^ 0b011;
}

inline constexpr bool CanBePlaced(const CellValue v, const Player p) {
  return v == TogglePlaceable(p) || v == NONE_BOTH;
}

inline const char* ToPlayerString(const Player v) {
  return v == DARK ? "Dark" : v == LIGHT ? "Light" : "None";
}

inline void PrintMove(std::ostream& os, const Move m, const BoardSize n) {
  os << "(" << (m / n) + 1 << ", " << (m % n) + 1 << ")";
}

constexpr const Move IllegalMove = -1;

template<BoardSize N>
class Board {
  static_assert(N >= 4, "N must be >= 4");
  static_assert(N % 2 == 0, "N must be even");
  static_assert(N <= 10, "N msut be <= 10");

 public:
  using Array = util::BitPack<3, N * N>;

  static constexpr Move GetMove(const int i, const int j) {
    return (i - 1) * N + j - 1;
  }

  Board()
      : array_(),
        current_player_(DARK),
        winner_(NONE),
        num_darks_(2),
        num_lights_(2) {
    StartingPosition();
  }

  const Array& array() const { return array_; }
  Player current_player() const { return current_player_; }
  Player winner() const { return winner_; }
  Move num_darks() const  { return num_darks_; }
  Move num_lights() const  { return num_lights_; }

  void Reset() {
    array_.clear();
    current_player_ = DARK;
    winner_ = NONE;
    num_darks_ = 2;
    num_lights_ = 2;
    StartingPosition();
  }

  bool IsFinished() const { return current_player_ == NONE; }

  bool IsDraw() const { return IsFinished() && winner_ == NONE; }

  bool IsLegalMove(const Move m) const {
    return !IsFinished() && m >= 0 && m < N * N && CanBePlaced(array_[m], current_player_);
  }

  int8_t GetDifference(const Player p) const {
    return p == DARK ? num_darks_ - num_lights_ : num_lights_ - num_darks_;
  }

  void Next(const int i, const int j) { Next(GetMove(i, j)); }

  void Next(const Move m) {
    assert(IsLegalMove(m));
    const auto p = current_player_;
    const auto q = GetOppositePlayer(p);
    auto& n_p = p == DARK ? num_darks_ : num_lights_;
    auto& n_q = p == DARK ? num_lights_ : num_darks_;
    array_[m] = p;
    ++n_p;
    const auto& lines = lines_[m];
    for (int d = 0; d < 4; ++d) {
      for (int e = 0; e < 2; ++e) {
        int state = 0;
        for (const auto i : lines[d][e]) {
          if (i < 0) break;
          const CellValue r = array_[i];
          if (state == 0) {
            if (r == q) {
              state = 1;
            } else {
              state = 3;
              break;
            }
          } else if (state == 1) {
            if (r == p) {
              state = 2;
              break;
            } else if (r != q) {
              state = 3;
              break;
            }
          }
        }
        if (state == 2) {
          for (const auto i : lines[d][e]) {
            if (array_[i] == q) {
              array_[i] = p;
              ++n_p;
              --n_q;
            } else {
              break;
            }
          }
        }
      }
    }
    bool p_has_legal_moves = false;
    bool q_has_legal_moves = false;
    for (Move mm = 0; mm < N * N; ++mm) {
      if (IsEmpty(array_[mm])) {
        array_[mm] = NONE;
        const auto& lines = lines_[mm];
        for (int d = 0; d < 4; ++d) {
          for (int e = 0; e < 2; ++e) {
            Player pp = NONE;
            Player qq = NONE;
            int state = 0;
            for (const auto i : lines[d][e]) {
              if (i < 0) break;
              const CellValue r = array_[i];
              if (state == 0) {
                if (!IsEmpty(r)) {
                  pp = GetOppositePlayer(r);
                  qq = r;
                  state = 1;
                } else {
                  state = 3;
                  break;
                }
              } else if (state == 1) {
                if (r == pp) {
                  state = 2;
                  break;
                } else if (r != qq) {
                  state = 3;
                  break;
                }
              }
            }
            if (state == 2) {
              array_[mm] = array_[mm] | TogglePlaceable(pp);
              if (pp == p) p_has_legal_moves = true;
              if (pp == q) q_has_legal_moves = true;
              if (array_[mm] == NONE_BOTH) goto after_line_check;
            }
          }
        }
        after_line_check: ;
      }
    }
    if (q_has_legal_moves) {
      current_player_ = q;
    } else if (p_has_legal_moves) {
      current_player_ = p;
    } else {
      current_player_ = NONE;
      winner_ = num_darks_ > num_lights_ ? DARK :
          num_darks_ < num_lights_ ? LIGHT : NONE;
    }
  }

  std::vector<Move> GetLegalMoves() const {
    std::vector<Move> moves;
    for (Move m = 0; m < N * N; ++m) {
      if (CanBePlaced(array_[m], current_player_)) moves.emplace_back(m);
    }
    return moves;
  }

 private:
  using Lines = util::Lines<int8_t, N, N - 1>;

  void StartingPosition() {
    const auto k = N / 2;
    array_[GetMove(k, k + 1)] = DARK;
    array_[GetMove(k + 1, k)] = DARK;
    array_[GetMove(k, k)] = LIGHT;
    array_[GetMove(k + 1, k + 1)] = LIGHT;

    array_[GetMove(k - 1, k)] = NONE_DARK;
    array_[GetMove(k, k - 1)] = NONE_DARK;
    array_[GetMove(k + 1, k + 2)] = NONE_DARK;
    array_[GetMove(k + 2, k + 1)] = NONE_DARK;

    array_[GetMove(k - 1, k + 1)] = NONE_LIGHT;
    array_[GetMove(k, k + 2)] = NONE_LIGHT;
    array_[GetMove(k + 1, k - 1)] = NONE_LIGHT;
    array_[GetMove(k + 2, k)] = NONE_LIGHT;
  }

  static const Lines lines_;
  Array array_;
  Player current_player_;
  Player winner_;
  Move num_darks_;
  Move num_lights_;
};

template<BoardSize N>
const typename Board<N>::Lines Board<N>::lines_ = util::BuildLines<int8_t, N, N - 1>();

template<BoardSize N>
std::ostream& operator<<(std::ostream& os, const Board<N>& board) {
  const auto& a = board.array();
  for (BoardSize i = 0; i < N; ++i) {
    for (BoardSize j = 0; j < N; ++j) {
      os << ToSymbol(a[i * N + j]);
    }
    os << std::endl;
  }
  os << "Dark: " << static_cast<int>(board.num_darks()) << std::endl;
  os << "Light: " << static_cast<int>(board.num_lights()) << std::endl;
  return os;
}

using History = std::vector<std::pair<Player, Move>>;

template<BoardSize N>
struct GameResult {
  Player winner;
  History history;
};

template<BoardSize N, class Dark, class Light, class Display>
void Play(Board<N>& board,
          Dark& p1,
          Light& p2,
          GameResult<N>& result,
          Display& display) {
  auto get_next_move = [&p1, &p2] (const Board<N>& board, const History& history) {
    return board.current_player() == DARK
        ? p1.GetNextMove(board, history)
        : p2.GetNextMove(board, history);
  };
  display.OnGameStart(board, result, p1.GetName(), p2.GetName());
  while (!board.IsFinished()) {
    display.OnBeforeMove(board, result);
    const auto m = get_next_move(board, result.history);
    result.history.emplace_back(board.current_player(), m);
    if (!board.IsLegalMove(m)) {
      display.OnIllegalMove(board, result);
      result.winner = GetOppositePlayer(board.current_player());
      display.OnGameFinish(board, result);
      return;
    }
    board.Next(m);
    display.OnAfterMove(board, result);
  }
  result.winner = board.winner();
  display.OnGameFinish(board, result);
}

template<BoardSize N>
struct GameTraits {
  using Board = othello::Board<N>;
  using Move = othello::Move;
  using Player = othello::Player;
  using History = othello::History;

  static constexpr auto MaxPos = N;

  static void PrintMove(std::ostream& os, const Move m) {
    othello::PrintMove(os, m, N);
  }

  static Move GetIllegalMove() { return IllegalMove; }
};

namespace player {

class Greedy {
 public:
  const char* GetName() const { return "Greedy"; }

  template<BoardSize N>
  Move GetNextMove(const Board<N>& board, const History& history) {
    Move best_move = IllegalMove;
    int8_t best_difference = -N * N;
    for (const auto m : board.GetLegalMoves()) {
      Board<N> b = board;
      b.Next(m);
      const auto d = b.GetDifference(board.current_player());
      if (d > best_difference) {
        best_move = m;
        best_difference = d;
      }
    }
    return best_move;
  }
};

}  // namespace player

namespace ui {

template<BoardSize N>
class BasicDisplay {
 public:
  BasicDisplay(std::ostream& os) : os_(os), verbosity_(2) {}

  void SetVerbosity(const int v) { verbosity_ = v; }

  void OnGameStart(const Board<N>& board,
                   const GameResult<N>& result,
                   const char* black_name,
                   const char* white_name) {
    os_ << "A new game has started." << std::endl;
    os_ << "Player 1 (Dark): " << black_name << std::endl;
    os_ << "Player 2 (Light): " << white_name << std::endl;
  }

  void OnBeforeMove(const Board<N>& board, const GameResult<N>& result) {
    if (verbosity_ < 2) return;
    os_ << std::endl
        << "Turn #" << result.history.size() + 1 << std::endl
        << ToPlayerString(board.current_player()) << "'s move" << std::endl
        << "Board before move:" << std::endl
        << board << std::endl;
  }

  void OnAfterMove(const Board<N>& board, const GameResult<N>& result) {
    if (verbosity_ < 1) return;
    os_ << ToPlayerString(result.history.back().first) << " at ";
    PrintMove(os_, result.history.back().second, N);
    os_ << std::endl;
  }

  void OnIllegalMove(const Board<N>& board, const GameResult<N>& result) {
    os_ << "Illegal move: ";
    PrintMove(os_, result.history.back().second, N);
    os_ << std::endl;
  }

  void OnGameFinish(const Board<N>& board, const GameResult<N>& result) {
    os_ << "The game has finished after " << result.history.size()
        << " moves." << std::endl
        << board << std::endl;
    if (result.winner == NONE) {
      os_ << "The game was a tie." << std::endl;
    } else {
      os_ << ToPlayerString(result.winner) << " has won the game." << std::endl;
    }
    if (verbosity_ >= 3) {
      os_ << "Moves:" << std::endl;
      size_t i = 1;
      for (const auto p : result.history) {
        os_ << "#" << i++ << ": " << ToPlayerString(p.first) << " at ";
        PrintMove(os_, p.second, N);
        os_ << std::endl;
      }
    }
  }

 private:
  std::ostream& os_;
  int verbosity_;
};

}  // namespace ui
}  // namespace othello
