#ifndef INFRARED_H
#define INFRARED_H

#include <stdint.h>

void rgb_to_nir_u8(
    const uint8_t* rgb,
    int rgb_stride,
    uint8_t* nir,
    int nir_stride,
    int width,
    int height,
    int channels,
    float wR,
    float wG,
    float wB,
    int use_gamma
);

void normalize_weights(float* wR, float* wG, float* wB);

#endif // INFRARED_H
