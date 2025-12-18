#ifndef BMP_IO_H
#define BMP_IO_H

#include <stdint.h>

int write_bmp_gray8(const char* path, int width, int height, const uint8_t* data, int stride);
int write_bmp_rgb24(const char* path, int width, int height, const uint8_t* data, int stride, int channels);

#endif // BMP_IO_H
