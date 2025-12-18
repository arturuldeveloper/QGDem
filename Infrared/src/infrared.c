#include "infrared.h"
#include <math.h>

static inline float srgb_to_linear(float v) {
    if (v <= 0.04045f) return v / 12.92f;
    return powf((v + 0.055f) / 1.055f, 2.4f);
}

static inline float clamp01(float x) {
    if (x < 0.0f) return 0.0f;
    if (x > 1.0f) return 1.0f;
    return x;
}

void normalize_weights(float* wR, float* wG, float* wB) {
    float s = (*wR) + (*wG) + (*wB);
    if (s <= 0.0f) { *wR = 1.0f; *wG = 0.0f; *wB = 0.0f; return; }
    *wR /= s; *wG /= s; *wB /= s;
}

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
) {
    normalize_weights(&wR, &wG, &wB);
    const int step = channels;
    for (int y = 0; y < height; ++y) {
        const uint8_t* src = rgb + y * rgb_stride;
        uint8_t* dst = nir + y * nir_stride;
        for (int x = 0; x < width; ++x) {
            int i = x * step;
            float r = src[i + 0] / 255.0f;
            float g = src[i + 1] / 255.0f;
            float b = src[i + 2] / 255.0f;
            if (use_gamma) {
                r = srgb_to_linear(r);
                g = srgb_to_linear(g);
                b = srgb_to_linear(b);
            }
            float v = wR * r + wG * g + wB * b;
            v = clamp01(v);
            dst[x] = (uint8_t)lrintf(v * 255.0f);
        }
    }
}
