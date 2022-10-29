#pragma once
#include "math.hpp"
#include "resource.hpp"
#include <array>
#include <misc/gl.hpp>
#include <simple_vector.hpp>

using buffer = simple_vector<unsigned char>;
template <typename T> using vector = simple_vector<T>;
using std::array;

#define HardCheck(X)                                                           \
  do {                                                                         \
    auto val = (X);                                                            \
    if (!(val)) {                                                              \
      std::cerr << "[[Hard check failed]] : " << __LINE__ << " -> " << #X      \
                << " evals to " << val << std::endl;                           \
      throw std::runtime_error("hard check error");                            \
    } /* else { std::cerr << "[[Hard check successful]] : " << __LINE__ << "   \
         -> " << #X << std::endl; } */                                         \
  } while (0)

#define SoftCheck(X, Y)                                                        \
  do {                                                                         \
    auto val = (X);                                                            \
    if (!(val)) {                                                              \
      Y                                                                        \
    } /* else { std::cerr << "[[Hard check successful]] : " << __LINE__ << "   \
         -> " << #X << std::endl; } */                                         \
  } while (0)

template <typename T> uint32_t hash(const T &object) {
  return std::hash<typename std::remove_reference<T>::type>()(object);
}

template <typename T, typename V> uint32_t binaryHash(const T &a, const V &b) {
  return hash(hash(a) * 3 + hash(b));
}

#pragma once
#define ENUM_OPERATORS(T)                                                      \
  inline T operator~(T a) { return (T) ~(int)a; }                              \
  inline T operator|(T a, T b) { return (T)((int)a | (int)b); }                \
  inline T operator&(T a, T b) { return (T)((int)a & (int)b); }                \
  inline T operator^(T a, T b) { return (T)((int)a ^ (int)b); }                \
  inline T &operator|=(T &a, T b) { return (T &)((int &)a |= (int)b); }        \
  inline T &operator&=(T &a, T b) { return (T &)((int &)a &= (int)b); }        \
  inline T &operator^=(T &a, T b) { return (T &)((int &)a ^= (int)b); }

using uint32_t = unsigned int;
using uint8_t = unsigned char;
