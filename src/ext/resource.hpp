#include <adapters/io.hpp>
namespace resource {
MemoryResource *createFromNullTerminatedString(const char *data,
                                               const char *id);
}
