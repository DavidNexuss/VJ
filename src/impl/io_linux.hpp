#include <controllers/io.hpp>

namespace shambhala {
struct LinuxIO : public shambhala::IIO {
  io_buffer readFile(const char *path) override;
  void freeFile(uint8_t *buffer) override;
};
} // namespace shambhala
