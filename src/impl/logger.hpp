#include <adapters/log.hpp>
#include <unordered_map>

namespace shambhala {
ILogger *log();
struct DefaultLogger : public shambhala::ILogger {
  void registerEvent(const char *type, int count = 1) override;
  void clearEvents() override;
  void log(const char *msg, int level = 0) override;
  void printEvents() override;

private:
  std::unordered_map<const char *, int> registeredEvents;
};

} // namespace shambhala
