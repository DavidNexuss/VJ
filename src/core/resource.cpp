#include "resource.hpp"
#include <cstring>

void IResource::signalUpdate() { acquisitionCount++; }
int IResource::updateCount() { return acquisitionCount; }

void IResource::set(io_buffer newbuffer) {
  io_buffer *old = read();
  delete[] old->data;
  old->data = newbuffer.data;
  old->length = newbuffer.length;
}

io_buffer io_buffer::create(const std::string &data) {
  unsigned char *buffer = new unsigned char[data.size() + 1];
  memcpy(buffer, data.c_str(), data.size());
  buffer[data.size()] = 0;
  io_buffer io;
  io.length = data.size();
  io.data = buffer;
  return io;
}
