#pragma once
#include <cmath>
#include <cstdint>
#include <initializer_list>
#include <iostream>
/**
 * Polymorphic capable vector with move semantics :
 * simple_vector<float> floatingValues;
 * simple_vector<uint32_t> vec(floatingValues.drop());
 * */

struct simple_vector_view {
  uint8_t *data;
  size_t size;
  size_t capacity;
  bool free = false;
};

struct simple_vector_span {
  uint8_t *data;
  size_t _size;
};

template <typename T> struct simple_vector_window : public simple_vector_span {
  size_t stride;
  simple_vector_window(const simple_vector_span &span)
      : simple_vector_span(span) {}

  T &operator[](size_t index) {
    return *((T *)&simple_vector_span::data[index * stride]);
  }

  size_t size() const { return _size / stride; }
};

template <typename T> class simple_vector {
  T *_data = nullptr;
  size_t _size = 0;
  size_t _capacity = 0;
  bool freeVector = false;
  bool debugFlag = false;

  void reserve(size_t newCapacity) {
    if (newCapacity < _capacity)
      return;

    T *newData = new T[newCapacity];
    for (size_t i = 0; i < _size; i++)
      newData[i] = _data[i];

    if (_data != nullptr)
      delete[] _data;
    _data = newData;
    _capacity = newCapacity;
  }

  void dealloc() {
    if (_data != nullptr)
      delete[] _data;
    _capacity = 0;
    _size = 0;
  }

  void copy(const simple_vector<T> &other) {
    dealloc();
    _data = new T[other.size()];
    for (size_t i = 0; i < other.size(); i++)
      _data[i] = other[i];
    _size = other.size();
    _capacity = _size;
  }

  void allocate(size_t newSize) {
    if (newSize <= _capacity) {
      _size = newSize;
      return;
    }

    if (_data != nullptr)
      delete[] _data;

    T *newData = new T[newSize];
    _size = newSize;
    _capacity = newSize;
    _data = newData;
  }

public:
  void allocateNew(size_t newSize) { allocate(newSize); }
  size_t push(const T &object) {
    if ((_size + 1) >= _capacity)
      reserve(std::max(size_t(2), _capacity * 2));
    _data[_size] = object;
    return _size++;
  }

  void resize(size_t newSize) {
    reserve(newSize);
    for (size_t i = _size; i < newSize; i++) {
      _data[i] = T();
    }
    _size = newSize;
  }
  size_t size() const { return _size; }

  simple_vector_view drop() {
    simple_vector_view view{(uint8_t *)_data, _size * sizeof(T), freeVector};
    _data = nullptr;
    _capacity = 0;
    _size = 0;
    view.free = freeVector;
    freeVector = false;

    return view;
  };

  simple_vector() {}
  simple_vector(size_t count) { resize(count); }
  simple_vector(simple_vector<T> &&other) {
    _data = other._data;
    _size = other._size;
    _capacity = other._capacity;
    freeVector = other.freeVector;

    other._data = nullptr;
    other.freeVector = false;
    other._size = 0;
    other._capacity = 0;
  }

  simple_vector(const T &object, int count) {
    reserve(count);
    for (int i = 0; i < count; i++) {
      _data[i] = object;
    }
    _size = count;
  }
  simple_vector(const simple_vector<T> &other) {
    copy(other);
    freeVector = other.freeVector;
  }

  template <typename K>
  simple_vector(K *data, int size, bool free = true)
      : _data((T *)data), _size(size * sizeof(K) / sizeof(T)) {
    freeVector = free;
  }

  simple_vector &operator=(const simple_vector<T> &other) {
    copy(other);
    freeVector = other.freeVector;
    return *this;
  }

  simple_vector(std::initializer_list<T> &&list) {
    reserve(list.size());
    size_t idx = 0;
    for (auto it = list.begin(); it != list.end(); it++) {
      _data[idx++] = *it;
    }
    _size = list.size();
  }

  simple_vector(const simple_vector_view &view) {
    _data = (T *)(view.data);
    _size = view.size / sizeof(T);
    freeVector = view.free;
  }
  simple_vector &operator=(const simple_vector_view &view) {
    dealloc();
    _data = (T *)(view.data);
    _size = view.size / sizeof(T);
    freeVector = view.free;
    return *this;
  }

  const T *data() const { return _data; }
  uint8_t *bindata() { return (uint8_t *)_data; }

  ~simple_vector() {
    if (_data != nullptr && !freeVector) {
      delete[] _data;
    }
  }

  T &operator[](size_t idx) {
    // if (debugFlag) std::cerr << "Access " << idx << std::endl;
    return _data[idx];
  }
  const T &operator[](size_t idx) const { return _data[idx]; }

  void clear() { _size = 0; }

  void fakeSize(int newSize) { _size = newSize; }

  simple_vector_span span() const {
    return {(uint8_t *)_data, _size * sizeof(T)};
  }

  void debug() { debugFlag = true; }

  const char *str() { return (const char *)_data; }

  void pop() { resize(size() - 1); }
};
