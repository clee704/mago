#include <chrono>
#include <fstream>
#include <iostream>
#include <random>
#include <vector>
#include "othello.hpp"
#include "player.hpp"
#include "util.hpp"

struct Result {
  int dark_time;
  int light_time;
  int dark_wins;
  int ties;
  int light_wins;
};

std::ostream& operator<<(std::ostream& os, const Result& r) {
  return os << r.dark_time << " "
            << r.light_time << " "
            << r.dark_wins << " "
            << r.ties << " "
            << r.light_wins;
}

int main() {
  constexpr const uint8_t N = 8;
  using GT = othello::GameTraits<N>;
  std::random_device rd;
  std::ofstream out("/dev/null");
  othello::ui::BasicDisplay<N> display(out);
  display.SetVerbosity(0);
  std::vector<Result> results;
  std::vector<int> times = {100, 200, 400, 1000, 2000, 3000};
  const auto rep = 10;
  for (const auto time1 : times) {
    for (const auto time2 : times) {
      Result r{time1, time2, 0, 0, 0};
      for (auto i = 0; i < rep; ++i) {
        const size_t seed1 = rd();
        std::cout << "Seed 1 = " << seed1 << std::endl;
        std::mt19937 rng1(seed1);
        player::GenericMCTS<GT> mcts1(rng1, time1);
        mcts1.SetBias(.4);
        const size_t seed2 = rd();
        std::cout << "Seed 2 = " << seed2 << std::endl;
        std::mt19937 rng2(seed2);
        player::GenericMCTS<GT> mcts2(rng2, time2);
        mcts2.SetBias(.4);
        othello::Board<N> b;
        othello::GameResult<N> result;
        othello::Play(b, mcts1, mcts2, result, display);
        if (result.winner == othello::DARK) {
          ++r.dark_wins;
        } else if (result.winner == othello::LIGHT) {
          ++r.light_wins;
        } else {
          ++r.ties;
        }
        std::cout << r << std::endl;
      }
      std::cout << "===" << std::endl;
      results.push_back(r);
      for (const auto r : results) {
        std::cout << r << std::endl;
      }
    }
  }
  return 0;
}
