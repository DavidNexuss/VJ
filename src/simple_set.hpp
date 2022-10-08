#include "simple_vector.hpp"

template <typename T> struct simple_set {
  simple_vector<T> data;
  simple_vector<int> iterateVector;
  simple_vector<int> toRemoveVector;
  int last = 1;
  int start = 0;

  int search(const T &object) {
    for (int i = 0; i < data.size(); i++) {
      if (data[i] == object)
        return i;
    }
    return -1;
  }

  void add(const T &obj) {
    data.push(obj);
    iterateVector.push(last);
    last++;
  }

  void remove(const T &obj) {
    int index = search(obj);
    if (index == -1)
      return;
    iterateVector[index] = iterateVector[iterateVector[index]];
  }

  template <typename F> void foreach (F &&f) {
    int i = start;
    while (i != last) {
      f(data[i]);
      i = iterateVector[i];
    }
  }
};
