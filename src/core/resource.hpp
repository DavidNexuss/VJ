#pragma once
#include <filesystem>
#include <string>
using uint8_t = unsigned char;
struct io_buffer {
  uint8_t *data;
  int length = 0;

  static io_buffer create(const std::string &data);
};

struct IResource {
  virtual io_buffer *read() = 0;
  virtual void write() {}
  virtual void set(io_buffer newbuffer);

  std::string resourcename;
  bool readOnly = false;

  int updateCount();
  void signalUpdate();

  int useCount = 0;

private:
  int acquisitionCount = 1;
};

template <typename T> struct ResourceHandlerAbstract {

  T *file() {
    if (!resource)
      return nullptr;
    if (lastAcquisition != resource->updateCount())
      return resource;
    return nullptr;
  }

  T *cleanFile() { return resource; }

  void acquire(T *resource) {
    this->resource = resource;
    lastAcquisition = 0;
  }
  void signalAck() { this->lastAcquisition = resource->updateCount(); }

private:
  int lastAcquisition = 0;
  T *resource = nullptr;
};

using ResourceHandler = ResourceHandlerAbstract<IResource>;
