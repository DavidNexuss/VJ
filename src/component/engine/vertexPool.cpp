#include "vertexPool.hpp"
#include <cstdio>
#include <iostream>

size_t EmptyAllocator::size() { return _size; }

void EmptyAllocator::resize(size_t size) { _size = size; }

uint8_t *EmptyAllocator::at(size_t position) {
  static uint8_t val;
  return &val;
}

size_t MemoryPool::acquireBucket(size_t size) {
  size_t current = findEmptyBucket(size);
  if (current == -1) {
    current = allocateBucket(size);
  }

  bucketsData[current].used = true;
  bucketsData[current].position = buckets[current].position;
  bucketsData[current].size = buckets[current].size;
  return current;
}

void MemoryPool::releaseBucket(size_t bucket) {
  bucketsData[bucket].used = false;
}

void MemoryPool::signalUpdate(size_t bucket) {
  size_t position = bucketsData[bucket].position;
  size_t size = bucketsData[bucket].size;

  bufferObject.pendingUpdate.push_back({position, size});
  bufferObject.pendingUpdate.back().buckets.push_back(bucket);
}

uint8_t *MemoryPool::getVertex(size_t bucket) {
  return allocator->at(bucketsData[bucket].position);
}

size_t MemoryPool::findEmptyBucket(size_t size) {
  for (int i = 0; i < buckets.size(); i++) {
    if (!bucketsData[buckets[i].bucketIdx].used && buckets[i].size >= size) {
      if (buckets[i].size > size) {
        BucketAllocation alloc;
        alloc.bucketIdx = bucketId++;
        alloc.size = buckets[i].size - size;
        alloc.position = buckets[i].position + size;

        buckets.resize(buckets.size() + 1);
        for (int j = (buckets.size() - 1); j > i && j > 0; j--) {
          buckets[j] = buckets[j - 1];
        }

        bucketsData[alloc.bucketIdx].used = false;
        buckets[i + 1] = alloc;

        buckets[i].size = size;

        print();
      }
      return buckets[i].bucketIdx;
    }
  }
  return -1;
}

size_t MemoryPool::allocateBucket(size_t size) {

  BucketAllocation alloc;
  alloc.bucketIdx = bucketId++;
  alloc.size = size;
  alloc.position = allocator->size();
  allocator->resize(allocator->size() + size);
  buckets.push_back(alloc);
  return alloc.bucketIdx;
}

void MemoryPool::cleanup() { merge(); }

void MemoryPool::merge() {
  static std::vector<int> toKeep;
  toKeep.clear();
  int i = 0;
  while (i < buckets.size()) {
    int j = i + 1;
    toKeep.push_back(i);
    if (!bucketsData[buckets[i].bucketIdx].used) {
      while (j < buckets.size() && !bucketsData[buckets[j].bucketIdx].used) {
        buckets[i].size += buckets[j].size;
        j++;
      }
    }
    i = j;
  }

  i = 0;
  for (int j = 0; j < toKeep.size(); j++) {
    buckets[i++] = buckets[toKeep[j]];
  }

  buckets.resize(toKeep.size());
}

Buffer *MemoryPool::getBufferObject() {
  bufferObject.memory = allocator->at(0);
  bufferObject.size = allocator->size();
  return &bufferObject;
}

void MemoryPool::print() {
  printf("[POOL]\n");
  printf("Current heap size: %lu\n", allocator->size());
  printf("Current heap ptr: %p\n", allocator->at(0));
  printf("Bucket count: %lu\n", bucketsData.size());
  printf("Bucket count useful: %lu\n\n", buckets.size());

  printf("Bucket \tidx \tused \tsize \tposition \tSIZE \tPOSITION\n\n");
  for (int i = 0; i < buckets.size(); i++) {

    printf("Bucket \t%lu \t%d \t%lu \t%lu \t%lu \t%lu\n", buckets[i].bucketIdx,
           bucketsData[buckets[i].bucketIdx].used, buckets[i].size,
           buckets[i].position, bucketsData[buckets[i].bucketIdx].size,
           bucketsData[buckets[i].bucketIdx].position);
  }

  bufferObject.mergeUpdates();
  getBufferObject();
  printf("[BUFFER OBJECT]\n");
  printf("Start %p Size %lu \n", bufferObject.memory, bufferObject.size);
  printf("Pending for update: \n");
  for (int i = 0; i < bufferObject.pendingUpdate.size(); i++) {
    printf(" > %d {%lu, %lu} { ", i, bufferObject.pendingUpdate[i].start,
           bufferObject.pendingUpdate[i].size);

    for (int w : bufferObject.pendingUpdate[i].buckets) {
      printf("%d ", w);
    }
    printf("}\n");
  }

  printf("\nChoose operation\n 1) Acquire bucket\n 2) Release bucket\n");
}

void MemoryPool::debug() {

  print();
  int operation;
  size_t size;
  size_t bucketIdx;
  while (std::cin >> operation) {
    switch (operation) {
    case 1:
      std::cin >> size;
      printf("Acquired: %lu: \n", acquireBucket(size));
      break;
    case 2:
      std::cin >> bucketIdx;
      releaseBucket(bucketIdx);
      break;
    case 3:
      merge();
      break;
    case 4:
      std::cin >> bucketIdx;
      signalUpdate(bucketIdx);
      break;
    case 5:
      bufferObject.pendingUpdate.clear();
      break;
    }

    print();
  }
}

#ifdef VERTEXPOOLDEBUG
int main() {
  MemoryPool pool(std::make_unique<EmptyAllocator>());
  pool.debug();
}
#endif
