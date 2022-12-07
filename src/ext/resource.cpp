#include "resource.hpp"
#include <cstring>

MemoryResource *resource::createFromNullTerminatedString(const char *data,
                                                         const char *id) {
  MemoryResource *result = new MemoryResource;
  result->buffer.data = (uint8_t *)data;
  result->buffer.length = strlen(data);
  result->resourcename = std::string(id);
  return result;
}
