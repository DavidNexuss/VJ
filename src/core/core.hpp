#pragma once
#include <array>
#include <misc/gl.hpp>
#include <vector>

using std::array;

#define HardCheck(X)                                                           \
  do {                                                                         \
    auto val = (X);                                                            \
    if (!(val)) {                                                              \
      std::cerr << "[[Hard check failed]] : " << __LINE__ << " -> " << #X      \
                << " evals to " << val << std::endl;                           \
      throw std::runtime_error("hard check error");                            \
    } /* else { std::cerr << "[[Hard check successful]] : " << __LINE__ << "   \
         -> " << #X << std::endl; } */                                         \
  } while (0)

#define SoftCheck(X, Y)                                                        \
  do {                                                                         \
    auto val = (X);                                                            \
    if (!(val)) {                                                              \
      Y                                                                        \
    } /* else { std::cerr << "[[Hard check successful]] : " << __LINE__ << "   \
         -> " << #X << std::endl; } */                                         \
  } while (0)

template <typename T> uint32_t hash(const T &object) {
  return std::hash<typename std::remove_reference<T>::type>()(object);
}

template <typename T, typename V> uint32_t binaryHash(const T &a, const V &b) {
  return hash(hash(a) * 3 + hash(b));
}

#pragma once
#define ENUM_OPERATORS(T)                                                      \
  inline T operator~(T a) { return (T) ~(int)a; }                              \
  inline T operator|(T a, T b) { return (T)((int)a | (int)b); }                \
  inline T operator&(T a, T b) { return (T)((int)a & (int)b); }                \
  inline T operator^(T a, T b) { return (T)((int)a ^ (int)b); }                \
  inline T &operator|=(T &a, T b) { return (T &)((int &)a |= (int)b); }        \
  inline T &operator&=(T &a, T b) { return (T &)((int &)a &= (int)b); }        \
  inline T &operator^=(T &a, T b) { return (T &)((int &)a ^= (int)b); }

using uint32_t = unsigned int;
using uint8_t = unsigned char;
#include <string>

struct io_buffer {
  uint8_t *data;
  int length = 0;

  io_buffer() {}
  io_buffer(uint8_t *_data, int _length) : data(_data), length(_length) {}

  static io_buffer create(const std::string &data);
};
using span = io_buffer;

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

struct EngineResource {
  void setConfigurationResource(IResource *resource);

  void save();
  void load();

  io_buffer serialized();
  inline void signalDirty() { dirty = true; }

  const char *configurationResourcePath();

protected:
  virtual io_buffer serialize() = 0;
  virtual void deserialize(io_buffer buffer) = 0;

private:
  io_buffer serialized_buffer;
  IResource *configurationResource = nullptr;
  bool dirty = true;
};
