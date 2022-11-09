#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <vector>
struct VertexPool;

struct BufferPointer {

  inline uint8_t *operator->() { return buffer->data() + position; }
  inline uint8_t &operator[](size_t idx) { return (*this)[position + idx]; }

private:
  std::vector<uint8_t> *buffer;
  size_t position;
  size_t bucketIdx;
  friend VertexPool;
};

struct VertexPool {
  virtual BufferPointer acquireBucket(size_t size);
  virtual void releaseBucket(BufferPointer ptr);
  virtual void signalUpdate(BufferPointer ptr);

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

  BufferPointer createPointer(size_t bucket);
  void destroyPointer(BufferPointer ptr);

  void merge();
  void print();

private:
  int findEmptyBucket(size_t size);
  int allocateBucket(size_t size);
};
