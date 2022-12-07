#pragma once
#include <string>
#include <typeinfo>

template <typename T> struct BaseComponent {

  void setName(const std::string &name) { this->instanceName = name; }
  const std::string &getName() {
    if (instanceName.empty()) {
      instanceName =
          std::string(typeid(T).name()) + " " + std::to_string(instanceId);
    }
    return this->instanceName;
  }

  BaseComponent() { instanceId = instanceCount++; }

private:
  static int instanceCount;
  int instanceId;
  std::string instanceName;
};
