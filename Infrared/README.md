# Infrared (RGB to NIR Approximation)

A tiny C project that converts an RGB image buffer to a single-channel "NIR" approximation. Since true NIR is not present in standard RGB, this uses a configurable weighted combination emphasizing red (optionally linearized via sRGB gamma).

## Build

Using CMake:

```powershell
cmake -S "c:\Users\James\Documents\Workspace\Infrared" -B "c:\Users\James\Documents\Workspace\Infrared\build"
cmake --build "c:\Users\James\Documents\Workspace\Infrared\build"
```

Artifacts:
- `infrared_demo` – CLI RGB→NIR converter (PPM→PGM)
- `test_infrared` – small unit test

## Usage

Input format: binary PPM (P6) with maxval 255. Output: binary PGM (P5).

```powershell
# Convert an image to PPM using ImageMagick
magick input.jpg -colorspace sRGB -depth 8 -compress none ppm:input.ppm

# Run converter (defaults: weights 0.75,0.25,0.0; no gamma)
."c:\Users\James\Documents\Workspace\Infrared\build\infrared_demo.exe" input.ppm output.pgm --weights=0.75,0.25,0.0 --gamma

# View or convert PGM back to PNG
magick output.pgm output.png
```

## Library API

```c
void rgb_to_nir_u8(const uint8_t* rgb, int rgb_stride,
                   uint8_t* nir, int nir_stride,
                   int width, int height, int channels,
                   float wR, float wG, float wB, int use_gamma);
```
- `rgb`/`nir`: row-major buffers
- `rgb_stride`/`nir_stride`: bytes per row
- `channels`: 3 (RGB) or 4 (RGBA; alpha ignored)
- `wR,wG,wB`: weights (auto-normalized)
- `use_gamma`: 1 to linearize sRGB before mixing

## Notes
- Weights default to 0.75 (R), 0.25 (G), 0.0 (B) to emphasize red.
- Gamma linearization often improves perceived tonal mapping.
- PPM/PGM are used to avoid external dependencies.
