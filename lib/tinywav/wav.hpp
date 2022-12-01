#include <stdint.h>
#include <string>

#pragma once
unsigned char *loadWAV(const std::string &filename, uint8_t &channels,
                       int &sampleRate, uint8_t &bitsPerSample, int &size);
