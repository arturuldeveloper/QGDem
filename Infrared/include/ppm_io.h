#ifndef PPM_IO_H
#define PPM_IO_H

#include <stdint.h>

typedef struct {
    int width;
    int height;
    int maxval;
    int channels; // 3 for PPM (RGB)
    int stride;   // width * channels
    uint8_t* data;
} PPMImage;

int read_ppm(const char* path, PPMImage* out);
int write_pgm(const char* path, int width, int height, int maxval, const uint8_t* data, int stride);
void free_ppm(PPMImage* img);

#endif // PPM_IO_H
