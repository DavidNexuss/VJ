#include "logger.hpp"
#include <iostream>
using namespace shambhala;

void DefaultLogger::registerEvent(const char *type, int count) {
  registeredEvents[type] += count;
}
void DefaultLogger::clearEvents() { registeredEvents.clear(); }
void DefaultLogger::log(const char *msg, int level) { puts(msg); }
void DefaultLogger::printEvents() {
  for (const auto &ent : registeredEvents) {
    std::cout << ent.first << " : " << ent.second << std::endl;
  }
}
