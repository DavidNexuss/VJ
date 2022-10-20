#include <adapters/serialize.hpp>
#include <unordered_map>

namespace shambhala {
struct StreamSerializer : public ISerializer {

  void serialize(const char *name, float value, int index = 0) override;
  void serialize(const char *name, glm::vec2 value, int index = 0) override;
  void serialize(const char *name, glm::vec3 value, int index = 0) override;

  io_buffer end() override;

  float deserializeFloat(const char *name, int index = 0) override;
  glm::vec2 deserializeVec2(const char *name, int index = 0) override;
  void deserializeBegin(io_buffer buffer) override;

private:
  std::string currentData = "";
  std::string deserializeData = "";

  struct DeserializeState {
    std::unordered_map<std::string, std::string> keyVals;
  };
};
} // namespace shambhala
