#include <core/core.hpp>
#include <vector>

namespace stdext {
template <typename T> void removeNShift(std::vector<T> &vector, int index) {
  if (index < 0)
    return;
  for (int i = index; i < (vector.size() - 1); i++) {
    vector[i] = vector[i + 1];
  }
  vector.resize(vector.size() - 1);
}

template <typename T> int linearFind(std::vector<T> &vector, const T &object) {
  for (int i = 0; i < vector.size(); i++) {
    if (vector[i] == object)
      return i;
  }
  return -1;
}
} // namespace stdext
