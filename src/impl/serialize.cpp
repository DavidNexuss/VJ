#include "serialize.hpp"
#include <cstring>
#include <string>

using namespace shambhala;

std::string stringify(float value) { return std::to_string(value); }

template <typename T> std::string stringify(T *arr, int count) {
  std::string data = "[";
  for (int i = 0; i < count; i++) {

    data += stringify(arr[count]);
    if (i < count - 1)
      data += ",";
  }
  return data + "]";
}
std::string stringify(glm::vec2 value) { return stringify(&value[0], 2); }
std::string stringify(glm::vec3 value) { return stringify(&value[0], 3); }

template <typename T>
std::string serializeM(const char *name, T value, int index) {

  return std::string(typeid(value).name()) + " " + std::string(name) + ":" +
         std::to_string(index) + ":" + stringify(value) + "\n";
}
void StreamSerializer::serialize(const char *name, float value, int index) {
  currentData += serializeM(name, value, index);
}
void StreamSerializer::serialize(const char *name, glm::vec2 value, int index) {
  currentData += serializeM(name, value, index);
}
void StreamSerializer::serialize(const char *name, glm::vec3 value, int index) {
  currentData += serializeM(name, value, index);
}

io_buffer StreamSerializer::end() {
  io_buffer data;

  uint8_t *buff = new uint8_t[currentData.size() + 1];
  memcpy(buff, currentData.c_str(), currentData.size());

  buff[currentData.size()] = 0;
  data.data = buff;
  data.length = currentData.size();
  currentData = "";
  return data;
}

void StreamSerializer::deserializeBegin(io_buffer buffer) {
  deserializeData = std::string((const char *)buffer.data);
}

float StreamSerializer::deserializeFloat(const char *name, int index) {}
