#include "ppm_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int read_token(FILE* f, char* buf, int max) {
    int c;
    // Skip whitespace and comments
    do {
        c = fgetc(f);
        if (c == '#') {
            // Skip comment line
            while (c != '\n' && c != EOF) c = fgetc(f);
        }
    } while (c != EOF && (c == ' ' || c == '\n' || c == '\r' || c == '\t'));

    if (c == EOF) return 0;

    int i = 0;
    buf[i++] = (char)c;
    while (i < max - 1) {
        c = fgetc(f);
        if (c == EOF || c == ' ' || c == '\n' || c == '\r' || c == '\t' || c == '#') {
            if (c == '#') {
                // Put back '#' for next read_token to handle comment
                ungetc(c, f);
            }
            break;
        }
        buf[i++] = (char)c;
    }
    buf[i] = '\0';
    return 1;
}

int read_ppm(const char* path, PPMImage* out) {
    memset(out, 0, sizeof(*out));
    FILE* f = fopen(path, "rb");
    if (!f) return 0;

    char tok[64];
    if (!read_token(f, tok, sizeof(tok))) { fclose(f); return 0; }
    if (strcmp(tok, "P6") != 0) { fclose(f); return 0; }
    if (!read_token(f, tok, sizeof(tok))) { fclose(f); return 0; }
    out->width = atoi(tok);
    if (!read_token(f, tok, sizeof(tok))) { fclose(f); return 0; }
    out->height = atoi(tok);
    if (!read_token(f, tok, sizeof(tok))) { fclose(f); return 0; }
    out->maxval = atoi(tok);
    if (out->width <= 0 || out->height <= 0 || out->maxval <= 0 || out->maxval > 255) { fclose(f); return 0; }

    // Consume single whitespace after header
    int c = fgetc(f);
    if (c == '\r') { c = fgetc(f); }
    if (c != '\n' && c != ' ' && c != '\t') {
        // If not whitespace, push back
        ungetc(c, f);
    }

    size_t count = (size_t)out->width * (size_t)out->height * 3u;
    out->data = (uint8_t*)malloc(count);
    if (!out->data) { fclose(f); return 0; }
    size_t read = fread(out->data, 1, count, f);
    fclose(f);
    if (read != count) { free(out->data); out->data = NULL; return 0; }
    out->channels = 3;
    out->stride = out->width * out->channels;
    return 1;
}

int write_pgm(const char* path, int width, int height, int maxval, const uint8_t* data, int stride) {
    FILE* f = fopen(path, "wb");
    if (!f) return 0;
    fprintf(f, "P5\n%d %d\n%d\n", width, height, maxval);
    for (int y = 0; y < height; ++y) {
        const uint8_t* row = data + y * stride;
        if ((int)fwrite(row, 1, width, f) != width) { fclose(f); return 0; }
    }
    fclose(f);
    return 1;
}

void free_ppm(PPMImage* img) {
    if (img && img->data) { free(img->data); img->data = NULL; }
}
