#include <adapters/io.hpp>

namespace shambhala {
struct LinuxIO : public shambhala::IIO {
  io_buffer internal_readFile(const std::string &path) override;
  void internal_freeFile(uint8_t *buffer) override;
};
} // namespace shambhala
