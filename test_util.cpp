#undef NDEBUG
#include <cassert>
#include <iostream>
#include "util.hpp"

void TestBitPack2() {
  util::BitPack<2, 9> bitpack;
  assert(bitpack[0] == 0);
  assert(bitpack[1] == 0);
  assert(bitpack[2] == 0);
  assert(bitpack[3] == 0);
  assert(bitpack[4] == 0);
  assert(bitpack[5] == 0);
  assert(bitpack[6] == 0);
  assert(bitpack[7] == 0);
  assert(bitpack[8] == 0);
  bitpack[0] = 1;
  assert(bitpack[0] == 1);
  assert(bitpack[1] == 0);
  assert(bitpack[2] == 0);
  assert(bitpack[3] == 0);
  assert(bitpack[4] == 0);
  assert(bitpack[5] == 0);
  assert(bitpack[6] == 0);
  assert(bitpack[7] == 0);
  assert(bitpack[8] == 0);
  bitpack[3] = 3;
  assert(bitpack[0] == 1);
  assert(bitpack[1] == 0);
  assert(bitpack[2] == 0);
  assert(bitpack[3] == 3);
  assert(bitpack[4] == 0);
  assert(bitpack[5] == 0);
  assert(bitpack[6] == 0);
  assert(bitpack[7] == 0);
  assert(bitpack[8] == 0);
  bitpack[2] = 1;
  bitpack[3] = 0;
  bitpack[7] = 2;
  bitpack[8] = 1;
  assert(bitpack[0] == 1);
  assert(bitpack[1] == 0);
  assert(bitpack[2] == 1);
  assert(bitpack[3] == 0);
  assert(bitpack[4] == 0);
  assert(bitpack[5] == 0);
  assert(bitpack[6] == 0);
  assert(bitpack[7] == 2);
  assert(bitpack[8] == 1);
}

void TestBitPack3() {
  util::BitPack<3, 5> bitpack;
  assert(bitpack[0] == 0);
  assert(bitpack[1] == 0);
  assert(bitpack[2] == 0);
  assert(bitpack[3] == 0);
  assert(bitpack[4] == 0);
  bitpack[2] = 3;
  assert(bitpack[0] == 0);
  assert(bitpack[1] == 0);
  assert(bitpack[2] == 3);
  assert(bitpack[3] == 0);
  assert(bitpack[4] == 0);
  bitpack[1] = 7;
  bitpack[2] = 1;
  bitpack[4] = 4;
  assert(bitpack[0] == 0);
  assert(bitpack[1] == 7);
  assert(bitpack[2] == 1);
  assert(bitpack[3] == 0);
  assert(bitpack[4] == 4);
}

int main() {
  TestBitPack2();
  TestBitPack3();
  std::cout << "OK" << std::endl;
}
