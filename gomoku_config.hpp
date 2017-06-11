#pragma once
#include "gomoku.hpp"

namespace gomoku {

inline void Config1(gomoku::Board<9>& b) {
  b.Next(5, 6);
  b.Next(5, 5);
  b.Next(6, 5);
  b.Next(6, 4);
  b.Next(7, 4);
  b.Next(4, 7);
  b.Next(8, 3);
  b.Next(9, 2);
  b.Next(6, 6);
  b.Next(4, 6);
  b.Next(7, 3);
  b.Next(3, 7);
  b.Next(2, 8);
  b.Next(4, 5);
  // .........
  // .......@.
  // ......O..
  // ....OOO..
  // ....O@...
  // ...O@@...
  // ..@@.....
  // ..@......
  // .O.......
  // black to move
  // correct move: (4, 4) or (4, 8)
}

inline void Config2(gomoku::Board<9>& b) {
  b.Next(5, 5);
  b.Next(6, 5);
  b.Next(5, 4);
  b.Next(5, 6);
  b.Next(7, 4);
  b.Next(4, 7);
  b.Next(6, 4);
  b.Next(8, 4);
  b.Next(4, 4);
  b.Next(3, 4);
  b.Next(4, 5);
  b.Next(3, 6);
  b.Next(4, 6);
  b.Next(3, 7);
  b.Next(3, 5);
  b.Next(2, 5);
  b.Next(7, 3);
  // .........
  // ....O....
  // ...O@OO..
  // ...@@@O..
  // ...@@O...
  // ...@O....
  // ..@@.....
  // ...O.....
  // .........
  // white to move
  // correct move: (8, 2)
}

inline void Config3(gomoku::Board<9>& b) {
  b.Next(5, 5);
  b.Next(4, 5);
  b.Next(5, 6);
  b.Next(5, 4);
  b.Next(4, 6);
  b.Next(3, 6);
  b.Next(2, 7);
  b.Next(6, 3);
  b.Next(7, 2);
  b.Next(6, 5);
  b.Next(6, 4);
  b.Next(7, 3);
  b.Next(5, 3);
  b.Next(7, 6);
  b.Next(3, 7);
  b.Next(2, 8);
  b.Next(8, 7);
  b.Next(7, 5);
  b.Next(5, 7);
  b.Next(7, 7);
  b.Next(7, 4);
  b.Next(4, 3);
  b.Next(3, 2);
  b.Next(4, 7);
  b.Next(3, 5);
  b.Next(7, 9);
  b.Next(7, 8);
  b.Next(2, 4);
  b.Next(4, 4);
  // .........
  // ...O..@O.
  // .@..@O@..
  // ..O@O@O..
  // ..@O@@@..
  // ..O@O....
  // .@O@OOO@O
  // ......@..
  // .........
  //
  // white to move
  // correct move: (2, 6) or (6, 2)
}

inline void Config4(gomoku::Board<9>& b) {
  b.Next(6, 5);
  b.Next(5, 5);
  b.Next(5, 6);
  b.Next(4, 4);
  b.Next(7, 4);
  b.Next(4, 7);
  b.Next(6, 6);
  b.Next(4, 6);
  b.Next(4, 5);
  b.Next(6, 4);
  b.Next(3, 7);
  b.Next(5, 3);
  b.Next(3, 4);
  b.Next(7, 3);
  b.Next(8, 2);
  // .........
  // .........
  // ...@..@..
  // ...O@OO..
  // ..O.O@...
  // ...O@@...
  // ..O@.....
  // .@.......
  // .........
  // white to move
  // correct move: (2, 3) or (6, 7)
}

}  // namespace gomoku
