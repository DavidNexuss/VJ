#include "vertexPool.hpp"
#include <cstdio>
#include <iostream>

BufferPointer VertexPool::acquireBucket(size_t size) {
  int current = findEmptyBucket(size);
  if (current >= 0) {
    return createPointer(current);
  }
  return createPointer(allocateBucket(size));
}

void VertexPool::releaseBucket(BufferPointer ptr) { destroyPointer(ptr); }

void VertexPool::signalUpdate(BufferPointer ptr) {}

int VertexPool::findEmptyBucket(size_t size) {
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

int VertexPool::allocateBucket(size_t size) {

  BucketAllocation alloc;
  alloc.bucketIdx = bucketId++;
  alloc.size = size;
  alloc.position = buffer.size();
  buffer.resize(buffer.size() + size);
  buckets.push_back(alloc);
  return alloc.bucketIdx;
}

BufferPointer VertexPool::createPointer(size_t bucket) {
  BufferPointer ptr;
  ptr.buffer = &buffer;
  ptr.position = buckets[bucket].position;
  bucketsData[bucket].used = true;
  return ptr;
}

void VertexPool::destroyPointer(BufferPointer ptr) {
  bucketsData[ptr.bucketIdx].used = false;
}

void VertexPool::cleanup() { merge(); }

void VertexPool::merge() {
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

void VertexPool::print() {
  printf("Current heap size: %lu\n", buffer.size());
  printf("Bucket count: %lu\n", bucketsData.size());
  printf("Bucket count useful: %lu\n\n", buckets.size());

  printf("Bucket \tidx \tused \tsize \tposition\n\n");
  for (int i = 0; i < buckets.size(); i++) {

    printf("Bucket \t%lu \t%d \t%lu \t%lu\n", buckets[i].bucketIdx,
           bucketsData[buckets[i].bucketIdx].used, buckets[i].size,
           buckets[i].position);
  }

  printf("\nChoose operation\n 1) Acquire bucket\n 2) Release bucket\n");
}

void VertexPool::debug() {

  std::unordered_map<int, BufferPointer> ptrs;

  print();
  int operation;
  size_t size;
  size_t bucketIdx;
  while (std::cin >> operation) {
    switch (operation) {
    case 1:
      std::cin >> size;
      printf("Acquired: %lu: \n", acquireBucket(size).bucketIdx);
      break;
    case 2:
      std::cin >> bucketIdx;
      BufferPointer ptr;
      ptr.bucketIdx = bucketIdx;
      releaseBucket(ptr);
      break;
    case 3:
      merge();
      break;
    }

    print();
  }
}
