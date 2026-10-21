// Paints TIGRIS's art from OCELOT's: the same cat-headed Jaguar, its jaguar's coat turned into a
// tiger's (orange fur, white where it was palest, black stripes instead of spots):
//
//   tigris <resources-dir> <ocelot-art-dir> <out-dir>
//
// reads  FIGHTR0.AF (where each of Jaguar's sprites sits), <ocelot-art-dir>/mMM_fFF.png
// writes <out-dir>/mMM_fFF.png, for tools/web/ocelot to build the robot from
//
// The stripes run across the robot where it stands, not across each picture, so that they stay
// put from one frame to the next as far as the limbs do: bands across the body, broken and
// tapering like a tiger's, a little wavy.
#include <math.h>
#include <png.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "formats/af.h"
#include "formats/error.h"
#include "formats/move.h"
#include "formats/sprite.h"
#include "utils/path.h"

#define ART_SCALE 4
#define STRIPE_PERIOD 34.0f // in art pixels: 8.5 of the game's

typedef struct {
    float r, g, b;
} rgbf;

static float lum(const unsigned char *p) {
    return 0.299f * p[0] + 0.587f * p[1] + 0.114f * p[2];
}

static float saturation(const unsigned char *p) {
    int mx = p[0] > p[1] ? (p[0] > p[2] ? p[0] : p[2]) : (p[1] > p[2] ? p[1] : p[2]);
    int mn = p[0] < p[1] ? (p[0] < p[2] ? p[0] : p[2]) : (p[1] < p[2] ? p[1] : p[2]);
    return mx ? (float)(mx - mn) / mx : 0;
}

// Ocelot's coat: its tans and browns (not the plating's grays, the fire's pure orange, the
// claws' off-white); its spots are the coat's near-black browns.
enum { OTHER, COAT, SPOT };

static int classify(const unsigned char *p) {
    if(p[3] < 128) {
        return OTHER;
    }
    float s = saturation(p), l = lum(p);
    bool brown = p[0] >= p[1] && p[1] >= p[2] && p[0] > p[2] + 8;
    if(!brown || s > 0.9f) {
        return OTHER;
    }
    if(l < 55) {
        return s > 0.3f ? SPOT : OTHER;
    }
    return s >= 0.25f ? COAT : OTHER;
}

// The tiger's fur by brightness, 0..1: a dark rust shadow, orange, and the palest parts white.
static rgbf fur(float t) {
    static const rgbf stops[] = {{70, 22, 4}, {168, 60, 8}, {230, 110, 22}, {252, 158, 58}, {255, 230, 196}};
    const int n = sizeof(stops) / sizeof(stops[0]) - 1;
    t = t < 0 ? 0 : t > 1 ? 1 : t;
    int i = (int)(t * n);
    i = i >= n ? n - 1 : i;
    float f = t * n - i;
    return (rgbf){stops[i].r + (stops[i + 1].r - stops[i].r) * f, stops[i].g + (stops[i + 1].g - stops[i].g) * f,
                  stops[i].b + (stops[i + 1].b - stops[i].b) * f};
}

static uint32_t hash2(int x, int y, uint32_t seed) {
    uint32_t h = (uint32_t)x * 374761393u + (uint32_t)y * 668265263u + seed * 2246822519u;
    h = (h ^ (h >> 13)) * 1274126177u;
    return h ^ (h >> 16);
}

// Smooth value noise, about one feature per `cell` pixels.
static float noise(float x, float y, float cell, uint32_t seed) {
    x /= cell;
    y /= cell;
    int ix = (int)floorf(x), iy = (int)floorf(y);
    float fx = x - ix, fy = y - iy;
    fx = fx * fx * (3 - 2 * fx);
    fy = fy * fy * (3 - 2 * fy);
    float v[4];
    for(int k = 0; k < 4; k++) {
        v[k] = (hash2(ix + (k & 1), iy + (k >> 1), seed) & 0xFFFF) / 65535.0f;
    }
    return (v[0] * (1 - fx) + v[1] * fx) * (1 - fy) + (v[2] * (1 - fx) + v[3] * fx) * fy;
}

// How much of a stripe covers the robot's point (x, y), 0..1.
static float stripe(float x, float y) {
    float wave = 9.0f * (noise(x, y, 40, 1) - 0.5f) + 0.12f * x; // wavy, and slanting a little
    float phase = (y + wave) / STRIPE_PERIOD;
    float d = fabsf(phase - floorf(phase) - 0.5f) * 2; // 0 at a band's middle, 1 between bands
    int band = (int)floorf(phase);
    // each band breaks into pieces that taper at their ends
    float width = 0.56f * (noise(x + band * 97.0f, band * 13.0f, 26, 2) * 1.6f - 0.3f);
    if(width <= 0) {
        return 0;
    }
    float edge = 1 - d / width;
    return edge <= 0 ? 0 : edge >= 0.25f ? 1 : edge / 0.25f;
}

static bool png_read(const char *fn, int *w, int *h, unsigned char **px) {
    png_image img;
    memset(&img, 0, sizeof(img));
    img.version = PNG_IMAGE_VERSION;
    if(!png_image_begin_read_from_file(&img, fn)) {
        return false;
    }
    img.format = PNG_FORMAT_RGBA;
    *px = malloc(PNG_IMAGE_SIZE(img));
    if(!png_image_finish_read(&img, NULL, *px, 0, NULL)) {
        free(*px);
        return false;
    }
    *w = img.width;
    *h = img.height;
    return true;
}

static bool png_write(const char *fn, int w, int h, const unsigned char *px) {
    png_image img;
    memset(&img, 0, sizeof(img));
    img.version = PNG_IMAGE_VERSION;
    img.width = w;
    img.height = h;
    img.format = PNG_FORMAT_RGBA;
    return png_image_write_to_file(&img, fn, 0, px, 0, NULL);
}

static void paint(unsigned char *px, int w, int h, int ox, int oy) {
    int *cls = malloc(sizeof(int) * w * h);
    for(int i = 0; i < w * h; i++) {
        cls[i] = classify(px + i * 4);
    }
    // the coat's brightness, its spots filled in from the coat around them
    float *l = malloc(sizeof(float) * w * h);
    for(int y = 0; y < h; y++) {
        for(int x = 0; x < w; x++) {
            int i = y * w + x;
            if(cls[i] == COAT) {
                l[i] = lum(px + i * 4);
            } else if(cls[i] == SPOT) {
                float sum = 0;
                int n = 0;
                for(int r = 4; r <= 16 && !n; r += 4) {
                    for(int dy = -r; dy <= r; dy++) {
                        for(int dx = -r; dx <= r; dx++) {
                            int sx = x + dx, sy = y + dy;
                            if(sx >= 0 && sy >= 0 && sx < w && sy < h && cls[sy * w + sx] == COAT) {
                                sum += lum(px + (sy * w + sx) * 4);
                                n++;
                            }
                        }
                    }
                }
                l[i] = n ? sum / n * 0.85f : 110;
            }
        }
    }
    // smoothed over a few pixels: the jaguar's mottling would read as a dirty tiger
    float *smooth = malloc(sizeof(float) * w * h);
    for(int y = 0; y < h; y++) {
        for(int x = 0; x < w; x++) {
            int i = y * w + x;
            if(cls[i] == OTHER) {
                continue;
            }
            float sum = 0;
            int n = 0;
            for(int dy = -3; dy <= 3; dy++) {
                for(int dx = -3; dx <= 3; dx++) {
                    int sx = x + dx, sy = y + dy;
                    if(sx >= 0 && sy >= 0 && sx < w && sy < h && cls[sy * w + sx] != OTHER) {
                        sum += l[sy * w + sx];
                        n++;
                    }
                }
            }
            smooth[i] = (sum / n) * 0.7f + l[i] * 0.3f;
        }
    }
    for(int y = 0; y < h; y++) {
        for(int x = 0; x < w; x++) {
            int i = y * w + x;
            if(cls[i] == OTHER) {
                continue;
            }
            rgbf c = fur((smooth[i] - 55) / 165.0f);
            float s = stripe(ox + x, oy + y);
            const rgbf black = {22, 12, 8};
            c = (rgbf){c.r + (black.r - c.r) * s, c.g + (black.g - c.g) * s, c.b + (black.b - c.b) * s};
            px[i * 4] = (unsigned char)(c.r + 0.5f);
            px[i * 4 + 1] = (unsigned char)(c.g + 0.5f);
            px[i * 4 + 2] = (unsigned char)(c.b + 0.5f);
        }
    }
    free(smooth);
    free(l);
    free(cls);
}

int main(int argc, char **argv) {
    if(argc != 4) {
        fprintf(stderr, "usage: %s <resources-dir> <ocelot-art-dir> <out-dir>\n", argv[0]);
        return 2;
    }
    char fn[1024];
    path p;
    sd_af_file af;
    sd_af_create(&af);
    snprintf(fn, sizeof(fn), "%s/FIGHTR0.AF", argv[1]);
    path_from_c(&p, fn);
    if(sd_af_load(&af, &p) != SD_SUCCESS) {
        fprintf(stderr, "cannot read %s\n", fn);
        return 1;
    }
    int frames = 0;
    for(int m = 0; m < 70; m++) {
        sd_move *move = sd_af_get_move(&af, m);
        if(!move) {
            continue;
        }
        for(int i = 0; i < sd_animation_get_sprite_count(move->animation) && i < 32; i++) {
            const sd_sprite *s = sd_animation_get_sprite(move->animation, i);
            int w, h;
            unsigned char *px;
            snprintf(fn, sizeof(fn), "%s/m%02d_f%02d.png", argv[2], m, i);
            if(!png_read(fn, &w, &h, &px)) {
                continue;
            }
            paint(px, w, h, s->pos.x * ART_SCALE, s->pos.y * ART_SCALE);
            snprintf(fn, sizeof(fn), "%s/m%02d_f%02d.png", argv[3], m, i);
            if(!png_write(fn, w, h, px)) {
                fprintf(stderr, "cannot write %s\n", fn);
                return 1;
            }
            free(px);
            frames++;
        }
    }
    sd_af_free(&af);
    printf("painted %d frames\n", frames);
    return 0;
}
