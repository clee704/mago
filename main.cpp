#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>

#define TERM_GRAY(s) "\e[38;5;242m" s "\e[0m"

namespace gomoku {

constexpr const uint8_t K = 5;

using BoardSize = uint8_t;
using Move = uint16_t;
using CellValue = int8_t;

enum : CellValue { BLACK = 1, WHITE = -1, NONE = 0 };

const char* ToSymbol(const CellValue v) {
  return v == BLACK ? TERM_GRAY("@") : v == WHITE ? "O" : ".";
}

inline int8_t GetOppositePlayer(const int8_t v) {
  return -v;
}

const char* ToPlayerString(const int8_t v) {
  return v == BLACK ? "Black" : v == WHITE ? "White" : "None";
}

void PrintMove(std::ostream& os, const Move m, const BoardSize n) {
  os << "(" << (m / n) + 1 << ", " << (m % n) + 1 << ")";
}

template<BoardSize N>
class Board {
  static_assert(N >= K, "N must be >= K");
  static_assert(N <= 181, "N must be <= 181");

 public:
  using Array = std::array<CellValue, N * N>;

  Board() : array_(), current_player_(BLACK), winner_(NONE) {}

  const Array& array() const { return array_; }
  int8_t current_player() const { return current_player_; }
  int8_t winner() const { return winner_; }

  void Reset() {
    array_.clear();
    current_player_ = BLACK;
    winner_ = NONE;
  }

  bool IsFinished() const { return current_player_ == NONE; }

  bool IsLegalMove(const Move m) const {
    return !IsFinished() && m < N * N && array_[m] == NONE;
  }

  void Next(const Move m) {
    assert(IsLegalMove(m));
    array_[m] = current_player_;
    current_player_ = GetOppositePlayer(current_player_);
    CheckWinner(m);
  }

  std::vector<Move> GetLegalMoves() const {
    std::vector<Move> moves;
    for (Move m = 0; m < N * N; ++m) {
      if (array_[m] == NONE) moves.emplace_back(m);
    }
    return moves;
  }

 private:
  void CheckWinner(const Move m) {
    const auto v = array_[m];
    if (CheckLine<0>(m, v)) return;
    if (CheckLine<1>(m, v)) return;
    if (CheckLine<2>(m, v)) return;
    if (CheckLine<3>(m, v)) return;
    for (const auto v : array_) {
      if (v == NONE) return;
    }
    // board is full
    current_player_ = NONE;
    winner_ = NONE;
  }

  template<int D>
  bool CheckLine(const Move m, const CellValue v) {
    BoardSize k = 1;
    for (const auto i : lines_[m][D][0]) {
      if (i >= 0 && array_[i] == v) {
        ++k;
      } else {
        break;
      }
    }
    for (const auto i : lines_[m][D][1]) {
      if (i >= 0 && array_[i] == v) {
        ++k;
      } else {
        break;
      }
    }
    if (k == K) {
      current_player_ = NONE;
      winner_ = v;
      return true;
    }
    return false;
  }

  using Lines = std::array<std::array<std::array<std::array<int16_t, K - 1>, 2>, 4>, N * N>;

  static Lines BuildLines() {
    Lines lines;
    for (Move m = 0; m < N * N; ++m) {
      for (int d = 0; d < 4; ++d) {
        for (int e = 0; e < 2; ++e) {
          lines[m][d][e].fill(-1);
          int bi = m / N;
          int bj = m % N;
          int tmp1 = (d | (d >> 1)) & 1;
          int tmp2 = (e << 1) - 1;
          int di = tmp1 * tmp2;
          int dj = (((d ^ tmp1) ^ 2) - 1) * tmp2;
          for (int k = 0, i = bi + di, j = bj + dj; k < 4; ++k, i += di, j += dj) {
            if (i < 0 || i >= N || j < 0 || j >= N) break;
            lines[m][d][e][k] = i * N + j;
          }
        }
      }
    }
    return lines;
  }

  static const Lines lines_;
  Array array_;
  int8_t current_player_;
  int8_t winner_;
};

template<BoardSize N>
const typename Board<N>::Lines Board<N>::lines_ = Board<N>::BuildLines();

template<BoardSize N>
std::ostream& operator<<(std::ostream& os, const Board<N>& board) {
  const auto& a = board.array();
  for (BoardSize i = 0; i < N; ++i) {
    for (BoardSize j = 0; j < N; ++j) {
      os << ToSymbol(a[i * N + j]);
    }
    os << std::endl;
  }
  return os;
}

template<BoardSize N>
struct GameResult {
  int8_t winner;
  std::vector<Move> history;
};

template<BoardSize N, class Black, class White, class Display>
void Play(Board<N>& board, Black& p1, White& p2, GameResult<N>& result, Display& display) {
  auto get_next_move = [&p1, &p2] (const Board<N>& board, const std::vector<Move>& history) {
    return board.current_player() == BLACK
        ? p1.GetNextMove(board, history)
        : p2.GetNextMove(board, history);
  };
  display.OnGameStart(board, result, p1.GetName(), p2.GetName());
  while (!board.IsFinished()) {
    display.OnBeforeMove(board, result);
    const auto m = get_next_move(board, result.history);
    result.history.emplace_back(m);
    if (!board.IsLegalMove(m)) {
      display.OnIllegalMove(board, result, m);
      result.winner = GetOppositePlayer(board.current_player());
      display.OnGameFinish(board, result);
      return;
    }
    board.Next(m);
    display.OnAfterMove(board, result, m);
  }
  result.winner = board.winner();
  display.OnGameFinish(board, result);
}

namespace player {

class Human {
 public:
  const char* GetName() const { return "Human"; }

  template<BoardSize N>
  Move GetNextMove(const Board<N>& board, const std::vector<Move>& history) {
    int i, j;
    Move m = 0;
    while (std::cin) {
      std::cout << "Enter next move: ";
      std::cin >> i >> j;
      if (i >= 1 && i <= N && j >= 1 && j <= N) {
        m = (i - 1) * N + (j - 1);
        if (board.IsLegalMove(m)) {
          break;
        } else {
          std::cerr << "Illegal move" << std::endl;
        }
      } else {
        std::cerr << "Illegal position: enter two numbers between 1 and " << static_cast<int>(N) << std::endl;
      }
    }
    if (!std::cin) {
      return 0;  // return illegal move to terminate the game
    }
    return m;
  }
};

class Random {
 public:
  Random() : rd_(), gen_(rd_()) {}

  const char* GetName() const { return "Random"; }

  template<BoardSize N>
  Move GetNextMove(const Board<N>& board, const std::vector<Move>& history) {
    const auto legal_moves = board.GetLegalMoves();
    assert(!legal_moves.empty());
    std::uniform_int_distribution<> dis(0, legal_moves.size() - 1);
    return legal_moves[dis(gen_)];
  }

 private:
  std::random_device rd_;
  std::mt19937 gen_;
};

class BasicMCTS {
};

}  // namespace player

namespace ui {

template<BoardSize N> 
class BasicDisplay {
 public:
  BasicDisplay(std::ostream& os) : os_(os), verbose_(false) {}

  void SetVerbose(bool b) { verbose_ = b; }

  void OnGameStart(const Board<N>& board,
                   const GameResult<N>& result,
                   const char* black_name,
                   const char* white_name) {
    os_ << "A new game has started." << std::endl;
    os_ << "Player 1 (Black): " << black_name << std::endl;
    os_ << "Player 2 (White): " << white_name << std::endl;
  }

  void OnBeforeMove(const Board<N>& board, const GameResult<N>& result) {
    os_ << std::endl
        << "Turn #" << result.history.size() + 1 << std::endl
        << ToPlayerString(board.current_player()) << "'s move" << std::endl
        << "Board before move:" << std::endl
        << board << std::endl;
  }

  void OnAfterMove(const Board<N>& board, const GameResult<N>& result, const Move m) {
    os_ << "Move: ";
    PrintMove(os_, m, N);
    os_ << std::endl;
  }

  void OnIllegalMove(const Board<N>& board, const GameResult<N>& result, const Move m) {
    os_ << "Illegal move: ";
    PrintMove(os_, m, N);
    os_ << std::endl;
  }

  void OnGameFinish(const Board<N>& board, const GameResult<N>& result) {
    os_ << "The game has finished." << std::endl;
    os_ << board << std::endl;
    if (board.winner() == NONE) {
      os_ << "The game was a draw." << std::endl;
    } else {
      os_ << ToPlayerString(board.winner()) << " has won the game." << std::endl;
    }
    if (verbose_) {
      os_ << "Moves:" << std::endl;
      size_t i = 1;
      for (const auto m : result.history) {
        os_ << "#" << i++ << ": ";
        PrintMove(os_, m, N);
        os_ << std::endl;
      }
    }
  }

 private:
  std::ostream& os_;
  bool verbose_;
};

}  // namespace ui
}  // namespace gomoku

int main() {
  gomoku::Board<15> b;
  gomoku::GameResult<15> result;
  gomoku::player::Random p1;
  gomoku::player::Random p2;
  gomoku::ui::BasicDisplay<15> display(std::cout);
  Play(b, p1, p2, result, display);
  return 0;
}
