#pragma once
#include <core/core.hpp>
namespace shambhala {

static int serial_type_int = 0;
static int serial_type_float = 1;
static int serial_type_vec2 = 2;
static int serial_type_vec3 = 3;
static int serial_type_str = 4;

struct ISerializer {

  virtual void serialize(const char *name, float value, int index = 0) = 0;
  virtual void serialize(const char *name, glm::vec2 value, int index = 0) = 0;
  virtual void serialize(const char *name, glm::vec3 value, int index = 0) = 0;
  virtual void serialize(const char *name, const char *value,
                         int index = 0) = 0;
  virtual io_buffer end() = 0;

  virtual float deserializeFloat(const char *name, int index = 0) = 0;
  virtual glm::vec2 deserializeVec2(const char *name, int index = 0) = 0;

  struct DeserializeEntry {
    std::string name;
    int type;
  };

  virtual void deserialize(io_buffer buffer) = 0;
};
} // namespace shambhala
