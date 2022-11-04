#include "io_std.hpp"

io_buffer shambhala::STDIO::internal_readFile(const std::string &path) { return io_buffer{}; }

void shambhala::STDIO::internal_freeFile(uint8_t *buffer) { delete[] buffer; }
