#include "bmp_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma pack(push, 1)
typedef struct {
    uint16_t bfType;      // 'BM'
    uint32_t bfSize;      // file size in bytes
    uint16_t bfReserved1; // 0
    uint16_t bfReserved2; // 0
    uint32_t bfOffBits;   // offset to pixel data
} BMPFILEHEADER;

typedef struct {
    uint32_t biSize;          // size of this header (40)
    int32_t  biWidth;
    int32_t  biHeight;        // positive for bottom-up
    uint16_t biPlanes;        // 1
    uint16_t biBitCount;      // 8 or 24
    uint32_t biCompression;   // 0 = BI_RGB
    uint32_t biSizeImage;     // image size (can be 0 for BI_RGB)
    int32_t  biXPelsPerMeter; // 0
    int32_t  biYPelsPerMeter; // 0
    uint32_t biClrUsed;       // 0
    uint32_t biClrImportant;  // 0
} BMPINFOHEADER;

typedef struct {
    uint8_t b;
    uint8_t g;
    uint8_t r;
    uint8_t a; // reserved
} RGBQUAD;
#pragma pack(pop)

static int write_headers(FILE* f, int width, int height, int bpp, uint32_t paletteBytes, uint32_t imageBytes) {
    BMPFILEHEADER fh;
    BMPINFOHEADER ih;
    fh.bfType = 0x4D42; // 'BM'
    fh.bfOffBits = sizeof(BMPFILEHEADER) + sizeof(BMPINFOHEADER) + paletteBytes;
    fh.bfSize = fh.bfOffBits + imageBytes;
    fh.bfReserved1 = 0;
    fh.bfReserved2 = 0;

    ih.biSize = sizeof(BMPINFOHEADER);
    ih.biWidth = width;
    ih.biHeight = height; // bottom-up
    ih.biPlanes = 1;
    ih.biBitCount = (uint16_t)bpp;
    ih.biCompression = 0; // BI_RGB
    ih.biSizeImage = imageBytes;
    ih.biXPelsPerMeter = 0;
    ih.biYPelsPerMeter = 0;
    ih.biClrUsed = 0;
    ih.biClrImportant = 0;

    if (fwrite(&fh, sizeof(fh), 1, f) != 1) return 0;
    if (fwrite(&ih, sizeof(ih), 1, f) != 1) return 0;
    return 1;
}

int write_bmp_gray8(const char* path, int width, int height, const uint8_t* data, int stride) {
    FILE* f = fopen(path, "wb");
    if (!f) return 0;
    uint32_t rowSize = (uint32_t)((width + 3) & ~3); // pad to 4 bytes
    uint32_t imageBytes = rowSize * (uint32_t)height;
    uint32_t paletteBytes = 256u * sizeof(RGBQUAD);

    if (!write_headers(f, width, height, 8, paletteBytes, imageBytes)) { fclose(f); return 0; }

    // Write 256-entry grayscale palette
    for (int i = 0; i < 256; ++i) {
        RGBQUAD q; q.b = (uint8_t)i; q.g = (uint8_t)i; q.r = (uint8_t)i; q.a = 0;
        if (fwrite(&q, sizeof(q), 1, f) != 1) { fclose(f); return 0; }
    }

    // Write bottom-up pixel rows
    uint8_t* rowbuf = (uint8_t*)malloc(rowSize);
    if (!rowbuf) { fclose(f); return 0; }
    for (int y = height - 1; y >= 0; --y) {
        const uint8_t* src = data + y * stride;
        memcpy(rowbuf, src, (size_t)width);
        memset(rowbuf + width, 0, rowSize - (uint32_t)width);
        if (fwrite(rowbuf, 1, rowSize, f) != rowSize) { free(rowbuf); fclose(f); return 0; }
    }
    free(rowbuf);
    fclose(f);
    return 1;
}

int write_bmp_rgb24(const char* path, int width, int height, const uint8_t* data, int stride, int channels) {
    FILE* f = fopen(path, "wb");
    if (!f) return 0;
    uint32_t rowSize = (uint32_t)(((uint32_t)width * 3u + 3u) & ~3u);
    uint32_t imageBytes = rowSize * (uint32_t)height;
    uint32_t paletteBytes = 0;

    if (!write_headers(f, width, height, 24, paletteBytes, imageBytes)) { fclose(f); return 0; }

    uint8_t* rowbuf = (uint8_t*)malloc(rowSize);
    if (!rowbuf) { fclose(f); return 0; }
    for (int y = height - 1; y >= 0; --y) {
        const uint8_t* src = data + y * stride;
        uint8_t* dst = rowbuf;
        for (int x = 0; x < width; ++x) {
            int i = x * channels;
            uint8_t r = src[i + 0];
            uint8_t g = src[i + 1];
            uint8_t b = src[i + 2];
            // BMP stores BGR
            *dst++ = b; *dst++ = g; *dst++ = r;
        }
        // pad
        memset(rowbuf + width * 3, 0, rowSize - (uint32_t)(width * 3));
        if (fwrite(rowbuf, 1, rowSize, f) != rowSize) { free(rowbuf); fclose(f); return 0; }
    }
    free(rowbuf);
    fclose(f);
    return 1;
}
