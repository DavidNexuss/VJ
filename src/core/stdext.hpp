#include <vector>
namespace stdext {
template <typename T> void removeNShift(std::vector<T> &vec, int index) {
  for (int i = index; i < (vec.size() - 1); i++) {
    vec[index] = vec[index + 1];
  }
  vec.resize(vec.size() - 1);
}
} // namespace stdext
