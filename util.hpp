#pragma once
#include <array>
#include <cassert>
#include <memory>
#include <utility>
#include <vector>

#define TERM_GRAY(s) "\e[38;5;242m" s "\e[0m"

namespace util {

template<class T>
inline T at_least_1(T x) {
  return x < 1 ? 1 : x;
}

template<class T, size_t N>
class FixedBulk {
 public:
  bool empty() const { return bulks_.empty() || bulks_[0].second[0] == 0; }

  size_t size() const {
    return bulks_.empty() ? 0 : (bulks_.size() - 1) * N + bulks_.back().second;
  }

  void clear() { bulks_.clear(); }

  T& Create() {
    if (bulks_.empty() || bulks_.back().second == N) {
      bulks_.emplace_back(std::unique_ptr<std::array<T, N>>{new std::array<T, N>{}}, 0);
    }
    return (*bulks_.back().first)[bulks_.back().second++];
  }

  T& operator[](const size_t i) const { return (*bulks_[i / N].first)[i % N]; }

 private:
  std::vector<std::pair<std::unique_ptr<std::array<T, N>>, size_t>> bulks_;
};

template<int B, size_t N>
class BitPack {
  static_assert(B > 0, "N must be > 0");
  static_assert(B < 8, "N must be < 8");

  static constexpr int8_t GetMask(const int b) { return (1 << b) - 1; }

 public:
  BitPack() : bits_() {}

  void clear() { bits_.clear(); }

  class ElementProxy {
   public:
    ElementProxy(BitPack& bitpack, size_t index) : bitpack_(bitpack), index_(index) {}

    operator uint8_t() const {
      return static_cast<const BitPack&>(bitpack_)[index_];
    }

    ElementProxy& operator=(uint8_t value) {
      auto& bits = bitpack_.bits_;
      const uint8_t v = value & GetMask(B);
      const size_t j = index_ * B / 8;
      const int k = index_ * B % 8;
      if (8 % B == 0) {
        // B == 1, 2, 4
        const auto s = (8 - B - k);
        bits[j] = (bits[j] & ~(GetMask(B) << s)) | (v << s);
      } else {
        // B == 3, 5, 6, 7
        if (8 - B >= k) {
          const auto s = (8 - B - k);
          bits[j] = (bits[j] & ~(GetMask(B) << s)) | (v << s);
        } else {
          const auto s = (B - (8 - k));
          const auto t = 8 - s;
          bits[j] = (bits[j] & ~(GetMask(B) >> s)) | (v >> s);
          bits[j + 1] = (bits[j + 1] & ~(GetMask(s) << t)) | ((v & GetMask(s)) << t);
        }
      }
      return *this;
    };

   private:
    BitPack& bitpack_;
    size_t index_;
  };

  uint8_t operator[](size_t i) const {
    const size_t j = i * B / 8;
    const int k = i * B % 8;
    if (8 % B == 0) {
      // B == 1, 2, 4
      return (bits_[j] >> (8 - B - k)) & GetMask(B);
    } else {
      // B == 3, 5, 6, 7
      if (8 - B >= k) {
        return (bits_[j] >> (8 - B - k)) & GetMask(B);
      } else {
        const auto s = (B - (8 - k));
        return ((bits_[j] & (GetMask(B) >> s)) << s) | ((bits_[j + 1] >> (8 - s)) & GetMask(s));
      }
    }
  }

  ElementProxy operator[](size_t i) {
    return ElementProxy(*this, i);
  }

 private:
  std::array<uint8_t, (B * N + 7) / 8> bits_;
  
  friend class ElementProxy;
};

inline void TestBitPack() {
  BitPack<2, 9> bitpack;
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

}  // namespace util
