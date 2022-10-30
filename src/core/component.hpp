#pragma once
#include <string>
namespace shambhala {
struct UIComponent {
  bool uiRender = false;
  bool uiSelected = false;
};

template <typename T> struct EngineComponent : public UIComponent {
  inline static int indexCount = 0;

  std::string stringName;
  int indexName = 0;

  EngineComponent() { indexName = indexCount++; }

  EngineComponent(const EngineComponent &other) {
    indexName = indexCount++;
    other.stringName = other.stringName + " " + std::to_string(indexName);
  }
  std::string &getName() {
    if (stringName.empty())
      stringName = std::to_string(indexName);
    return stringName;
  }

  virtual void setName(const char *name) {
    if (name != nullptr)
      stringName = name;
  }
};

struct Updatable {
  inline void signalUpdate() { _needsUpdate = true; }
  inline void signalAck() { _needsUpdate = false; }
  inline bool needsUpdate() { return _needsUpdate; }

private:
  bool _needsUpdate = true;
};
} // namespace shambhala
