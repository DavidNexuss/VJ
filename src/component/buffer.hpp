#include <cstddef>
#include <cstdint>
#include <vector>

struct BufferRegion {
  size_t start;
  size_t size;

  std::vector<int> buckets;
};

struct Buffer {
  uint8_t *memory;
  size_t size;
  std::vector<BufferRegion> pendingUpdate;

  void mergeUpdates();
};
