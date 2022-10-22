#include "serialize.hpp"
#include <cstring>
#include <string>

using namespace shambhala;
const char *delimeter = "~";

std::string stringify(float value) { return std::to_string(value); }

template <typename T> std::string stringify(T *arr, int count) {
  std::string data = "[";
  for (int i = 0; i < count; i++) {

    data += stringify(arr[i]);
    if (i < count - 1)
      data += ",";
  }
  return data + "]";
}
std::string stringify(glm::vec2 value) { return stringify(&value[0], 2); }
std::string stringify(glm::vec3 value) { return stringify(&value[0], 3); }
std::string stringify(const char *value) { return std::string(value); }

template <typename T>
std::string serializeM(int type, const char *name, T value, int index) {

  return stringify(type) + delimeter + std::string(name) + delimeter +
         stringify(value) + "\n";
}
void StreamSerializer::serialize(const char *name, float value, int index) {
  currentData += serializeM(serial_type_int, name, value, index);
}
void StreamSerializer::serialize(const char *name, glm::vec2 value, int index) {
  currentData += serializeM(serial_type_vec2, name, value, index);
}
void StreamSerializer::serialize(const char *name, glm::vec3 value, int index) {
  currentData += serializeM(serial_type_vec3, name, value, index);
}
void StreamSerializer::serialize(const char *name, const char *value,
                                 int index) {
  currentData += serializeM(serial_type_str, name, value, index);
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
#include <string>
#include <vector>
using namespace std;

std::vector<string> string_burst(string str, char delimeter) {
  // condition: works with only single character delimeter, like $,_,space,etc.
  std::vector<string> words;
  int n = str.length();
  for (int i = 0; i < n; i++) {
    int j = i;
    while (str[i] != delimeter && i < n)
      i++;
    string temp = str.substr(j, i - j);
    words.push_back(temp);
  }
  return words;
}

void StreamSerializer::deserialize(io_buffer buffer) {
  this->deserializeState = DeserializeState{};

  std::string data = std::string((const char *)buffer.data);
  int count = 0;
  for (auto &str : string_burst(data, '\n')) {
    std::vector<std::string> fields = string_burst(str, ':');
    DeserializedData data;
    data.data = fields[2];
    data.name = fields[1];
    data.type = std::stoi(fields[0]);

    this->deserializeState.data.push(data);
    this->deserializeState.keysToData[data.name] = count++;
  }
}

float StreamSerializer::deserializeFloat(const char *name, int index) {
  int i = this->deserializeState.keysToData[std::string(name)];
  return std::stof(this->deserializeState.data[i].data);
}
glm::vec2 StreamSerializer::deserializeVec2(const char *name, int index) {}
