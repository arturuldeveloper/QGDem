#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "infrared.h"

static int approx(uint8_t a, uint8_t b, int tol) { return (int)abs((int)a - (int)b) <= tol; }

int main(void) {
    int width = 2, height = 2, channels = 3;
    uint8_t rgb[2*2*3] = {
        255, 0, 0,   0, 255, 0,
        0, 0, 255,   255, 255, 255
    };
    uint8_t nir[4];
    rgb_to_nir_u8(rgb, width*channels, nir, width, width, height, channels, 0.75f, 0.25f, 0.0f, 0);
    if (!approx(nir[0], 191, 1)) { fprintf(stderr, "Test failed at (0,0): %u\n", nir[0]); return 1; }
    if (!approx(nir[1], 64, 1)) { fprintf(stderr, "Test failed at (1,0): %u\n", nir[1]); return 1; }
    if (!approx(nir[2], 0, 1)) { fprintf(stderr, "Test failed at (0,1): %u\n", nir[2]); return 1; }
    if (!approx(nir[3], 255, 1)) { fprintf(stderr, "Test failed at (1,1): %u\n", nir[3]); return 1; }
    printf("infrared_basic: OK\n");
    return 0;
}
