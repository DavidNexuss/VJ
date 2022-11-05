#include "io_std.hpp"
#include <fstream>
#include <sstream>

io_buffer shambhala::STDIO::internal_readFile(const std::string &path) {
  std::ifstream input(path);

  if (input) {
    io_buffer result;
    std::ostringstream ss;
    ss << input.rdbuf();
    std::string data = ss.str();
    result.data = new uint8_t[data.size() + 1];
    result.length = data.size() + 1;
    for (int i = 0; i < data.size(); i++) {
      result.data[i] = data[i];
    }
    result.data[data.size()] = 0;
    return result;

  } else {
    return io_buffer();
  }
}
void shambhala::STDIO::internal_freeFile(uint8_t *buffer) { delete[] buffer; }
