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

void EngineResource::save() {
  if (configurationResource == nullptr || dirty == false)
    return;

  io_buffer data = serialize();
  *configurationResource->read() = data;
  configurationResource->write();
  dirty = false;
}

void EngineResource::load() {
  if (configurationResource == nullptr)
    return;
  deserialize(*configurationResource->read());
}

void EngineResource::setConfigurationResource(IResource *resource) {
  configurationResource = resource;
}

io_buffer EngineResource::serialized() {
  if (serialized_buffer.data != nullptr)
    delete[] serialized_buffer.data;

  dirty = false;
  return serialized_buffer = serialize();
}

const char *EngineResource::configurationResourcePath() {
  if (configurationResource == nullptr)
    return nullptr;
  return configurationResource->resourcename.c_str();
}
