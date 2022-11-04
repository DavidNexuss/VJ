#ifdef NIX
#include "io_linux.hpp"
#include <adapters/log.hpp>
#include <fcntl.h>
#include <sys/stat.h>

#include <unistd.h>

static int getFileSize(const char *path) {
  struct stat st;
  int ret = stat(path, &st);
  return st.st_size * (ret == 0);
}

io_buffer shambhala::LinuxIO::internal_readFile(const std::string &path_s) {

  const char *path = path_s.c_str();
  int fileSize = getFileSize(path);
  if (fileSize == 0) {
    return io_buffer();
  }
  uint8_t *fileBuffer = new uint8_t[fileSize + 1];
  int n;
  int idx = 0;
  int fd = open(path, O_RDONLY);
  while (idx < fileSize && (n = read(fd, &fileBuffer[idx], fileSize - idx))) {
    idx += n;
  }
  fileBuffer[fileSize] = 0;
  return {fileBuffer, fileSize};
}

void shambhala::LinuxIO::internal_freeFile(uint8_t *buffer) { delete[] buffer; }
#endif
