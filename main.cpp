#include <chrono>
#include <iostream>
#include "algo.hpp"
#include "gomoku.hpp"
#include "gomoku_config.hpp"
#include "util.hpp"

int main() {
  constexpr const uint8_t N = 11;
  gomoku::ui::BasicDisplay<N> display(std::cout);
  display.SetVerbosity(2);
  gomoku::player::Random random;
  gomoku::player::Human human;
  algo::GenericMCTS<gomoku::GameTraits<N>, true> mcts1(std::chrono::seconds(5));
  mcts1.SetBias(.4);
  algo::GenericMCTS<gomoku::GameTraits<N>, true> mcts2(std::chrono::seconds(5));
  mcts2.SetBias(.4);
  gomoku::Board<N> b;
  gomoku::GameResult<N> result;
  gomoku::Play(b, human, mcts2, result, display);
  return 0;
}
