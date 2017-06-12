#include <chrono>
#include <iostream>
#include <random>
#include "algo.hpp"
#include "gomoku.hpp"
#include "gomoku_config.hpp"
#include "util.hpp"

int main() {
  constexpr const uint8_t N = 11;
  using GT = gomoku::GameTraits<N>;
  std::random_device rd;
  const size_t seed1 = rd();
  const size_t seed2 = rd();
  const size_t seed3 = rd();
  const size_t seed4 = rd();
  std::cout << "Seed 1 = " << seed1 << std::endl
            << "Seed 2 = " << seed2 << std::endl
            << "Seed 3 = " << seed3 << std::endl
            << "Seed 4 = " << seed4 << std::endl;
  std::mt19937 rng1(seed1);
  std::mt19937 rng2(seed2);
  std::mt19937 rng3(seed3);
  std::mt19937 rng4(seed4);
  gomoku::ui::BasicDisplay<N> display(std::cout);
  display.SetVerbosity(2);
  algo::Random<GT> random1(rng1);
  algo::Random<GT> random2(rng2);
  gomoku::player::Human human;
  algo::GenericMCTS<GT, true> mcts1(rng3, std::chrono::seconds(30));
  mcts1.SetBias(.4);
  algo::GenericMCTS<GT, true> mcts2(rng4, std::chrono::seconds(30));
  mcts2.SetBias(.4);
  gomoku::Board<N> b;
  gomoku::GameResult<N> result;
  gomoku::Play(b, mcts1, mcts2, result, display);
  return 0;
}
