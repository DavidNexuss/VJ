#pragma once
namespace shambhala {
struct ILogger {
  virtual void log(const char *msg, int level = 0) = 0;
  virtual void registerEvent(const char *type, int count = 1) = 0;
  virtual void clearEvents() = 0;
  virtual void printEvents() = 0;
};

ILogger *log();
} // namespace shambhala

#ifdef DEBUG
#define LOGS(severity, fmt, ...)                                               \
  do {                                                                         \
    char buffer[4096];                                                         \
    sprintf(buffer, "[%s : %d] " fmt, __FILE__, __LINE__, __VA_ARGS__);        \
    shambhala::log()->log(buffer, severity);                                   \
  } while (0)

#define LOG(...) LOGS(0, __VA_ARGS__)
#else
#define LOGS(...)                                                              \
  do {                                                                         \
  } while (0)
#define LOG(...)                                                               \
  do {                                                                         \
  } while (0)
#endif
