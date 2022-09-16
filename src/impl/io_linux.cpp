#include "io_linux.hpp"
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

static int getFileSize(const char *path) {
  struct stat st;
  stat(path, &st);
  return st.st_size;
}

io_buffer shambhala::LinuxIO::internal_readFile(const char *path) {

  int fileSize = getFileSize(path);
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
