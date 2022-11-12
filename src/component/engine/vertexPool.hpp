#include "../buffer.hpp"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

struct VertexPool;

struct IAllocator {
  virtual void resize(size_t size) = 0;
  virtual size_t size() = 0;
  virtual uint8_t *at(size_t position) = 0;
};

struct EmptyAllocator : public IAllocator {
  void resize(size_t size) override;
  size_t size() override;
  uint8_t *at(size_t position) override;

private:
  size_t _size;
};

struct VectorAllocator : public IAllocator {
  void resize(size_t size) override;
  size_t size() override;
  uint8_t *at(size_t position) override;
};

struct MemoryPool {

  MemoryPool(std::unique_ptr<IAllocator> &&alloc)
      : allocator(std::move(alloc)) {}

  virtual size_t acquireBucket(size_t size);
  virtual void releaseBucket(size_t bucket);
  virtual void signalUpdate(size_t bucket);

  virtual void cleanup();

  void debug();

  uint8_t *getVertex(size_t bucket);
  Buffer *getBufferObject();

protected:
  struct BucketAllocation {
    size_t bucketIdx;
    size_t position;
    size_t size;
  };

  struct Bucket {
    bool used;
    size_t position;
    size_t size;
  };

  size_t bucketId = 0;

  std::unique_ptr<IAllocator> allocator;

  std::vector<BucketAllocation> buckets;
  std::unordered_map<size_t, Bucket> bucketsData;

  void merge();
  void print();

private:
  size_t findEmptyBucket(size_t size);
  size_t allocateBucket(size_t size);
  Buffer bufferObject;
};
