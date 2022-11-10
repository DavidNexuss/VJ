#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <vector>
struct VertexPool;

struct VertexPool {
  virtual size_t acquireBucket(size_t size);
  virtual void releaseBucket(size_t bucket);
  virtual void signalUpdate(size_t bucket);

  virtual void cleanup();

  void debug();

protected:
  struct BucketAllocation {
    size_t bucketIdx;
    size_t position;
    size_t size;
  };

  struct Bucket {
    bool used;
  };

  size_t bucketId = 0;
  std::vector<uint8_t> buffer;

  std::vector<BucketAllocation> buckets;
  std::unordered_map<size_t, Bucket> bucketsData;

  void merge();
  void print();

private:
  size_t findEmptyBucket(size_t size);
  size_t allocateBucket(size_t size);
};
