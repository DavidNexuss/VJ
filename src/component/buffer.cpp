#include "buffer.hpp"
#include <algorithm>

void Buffer::mergeUpdates() {
  std::sort(pendingUpdate.begin(), pendingUpdate.end(),
            [](auto &lhs, auto &rhs) { return lhs.start < rhs.start; });

  static std::vector<int> toKeep;
  toKeep.clear();

  int i = 0;
  while (i < pendingUpdate.size()) {
    toKeep.push_back(i);
    int j = i + 1;
    while (j < pendingUpdate.size() &&
           (pendingUpdate[i].start + pendingUpdate[i].size) >=
               pendingUpdate[j].start) {
      pendingUpdate[i].size =
          std::max(pendingUpdate[i].size, pendingUpdate[j].start +
                                              pendingUpdate[j].size -
                                              pendingUpdate[i].start);
      for (int w : pendingUpdate[j].buckets) {
        pendingUpdate[i].buckets.push_back(w);
      }

      j = j + 1;
    }

    i = j;
  }

  int j = 0;
  for (int i = 0; i < toKeep.size(); i++) {
    pendingUpdate[j++] = pendingUpdate[toKeep[i]];
  }

  pendingUpdate.resize(j);
}
