#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "infrared.h"
#include "ppm_io.h"
#include "bmp_io.h"

static void parse_weights(const char* s, float* r, float* g, float* b) {
    *r = 0.75f; *g = 0.25f; *b = 0.0f;
    if (!s) return;
    char buf[128];
    strncpy(buf, s, sizeof(buf));
    buf[sizeof(buf)-1] = '\0';
    char* p = strtok(buf, ",");
    if (p) *r = (float)atof(p);
    p = strtok(NULL, ",");
    if (p) *g = (float)atof(p);
    p = strtok(NULL, ",");
    if (p) *b = (float)atof(p);
}

typedef enum { OUT_PGM, OUT_BMP } OutFmt;

int main(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <input.ppm> <output.(pgm|bmp)> [--weights=R,G,B] [--gamma] [--format=pgm|bmp] [--save-rgb-bmp=path]\n", argv[0]);
        return 1;
    }
    const char* in_path = argv[1];
    const char* out_path = argv[2];
    float wR = 0.75f, wG = 0.25f, wB = 0.0f;
    int use_gamma = 0;
    OutFmt outfmt = OUT_PGM;
    const char* save_rgb_bmp = NULL;
    for (int i = 3; i < argc; ++i) {
        if (strncmp(argv[i], "--weights=", 10) == 0) {
            parse_weights(argv[i] + 10, &wR, &wG, &wB);
        } else if (strcmp(argv[i], "--gamma") == 0) {
            use_gamma = 1;
        } else if (strncmp(argv[i], "--format=", 9) == 0) {
            const char* f = argv[i] + 9;
            if (strcmp(f, "bmp") == 0) outfmt = OUT_BMP;
            else outfmt = OUT_PGM;
        } else if (strncmp(argv[i], "--save-rgb-bmp=", 15) == 0) {
            save_rgb_bmp = argv[i] + 15;
        }
    }

    PPMImage img;
    if (!read_ppm(in_path, &img)) {
        fprintf(stderr, "Failed to read PPM: %s\n", in_path);
        return 2;
    }

    uint8_t* nir = (uint8_t*)malloc((size_t)img.width * (size_t)img.height);
    if (!nir) { free_ppm(&img); fprintf(stderr, "Out of memory\n"); return 3; }

    rgb_to_nir_u8(img.data, img.stride, nir, img.width, img.width, img.height, img.channels, wR, wG, wB, use_gamma);

    int ok = 1;
    if (outfmt == OUT_PGM) {
        ok = write_pgm(out_path, img.width, img.height, 255, nir, img.width);
        if (!ok) fprintf(stderr, "Failed to write PGM: %s\n", out_path);
    } else {
        ok = write_bmp_gray8(out_path, img.width, img.height, nir, img.width);
        if (!ok) fprintf(stderr, "Failed to write BMP (gray): %s\n", out_path);
    }

    if (ok && save_rgb_bmp) {
        if (!write_bmp_rgb24(save_rgb_bmp, img.width, img.height, img.data, img.stride, img.channels)) {
            fprintf(stderr, "Failed to write RGB BMP: %s\n", save_rgb_bmp);
        }
    }

    if (!ok) { free(nir); free_ppm(&img); return 4; }

    free(nir);
    free_ppm(&img);
    return 0;
}
