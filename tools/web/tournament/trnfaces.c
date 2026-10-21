// Adds the World Championship's pilots (WORLD.TRN) to the select and VS screens:
//
//   trnfaces <resources-dir>
//
// reads  WORLD.TRN, WORLD.PIC, MELEE.BK, VS.BK, ARENA3.BK
// writes MELEE.BK with their grid icons (animation 3) and big faces (animation 4), and
//        VS.BK with their big faces (animation 4), at sprite indices 13.. (the engine's
//        PILOT_TOURNAMENT_FIRST, after Trump's and Musk's), one per pilot in WORLD.TRN's order
//        but the original ten (they are in it too), then Fire and Ice (PILOT_FIRE, PILOT_ICE).
//
// Fire and Ice, the single player game's secret opponents, have no photo: their faces are Nicoli's
// (half red, half blue already) in the colors the Fire Pit gives their robots.
//
// The others' faces are the tournament's own photos, cropped to the original pilots' 57x57 like
// theirs, and the icons (51x36, on black) close-ups of them. A photo's first 48 colors are
// its own, the rest the VS screen's; both are mapped to the colors the original faces are
// drawn in (0xA1..0xEF of each scene's palette), so the menus' dimmed icons (0x01..0x4F,
// half as bright) work on them as on the originals'.
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <string.h>

#include "formats/bk.h"
#include "formats/bkanim.h"
#include "formats/error.h"
#include "formats/pic.h"
#include "formats/sprite.h"
#include "formats/tournament.h"
#include "formats/vga_image.h"
#include "utils/path.h"

#define FIRST_PILOT 13
#define ICONS 3 // MELEE.BK: grid icons, one per pilot
#define FACES 4 // MELEE.BK and VS.BK: big faces, one per pilot
#define FACE_SIZE 57
#define ICON_W 51
#define ICON_H 36
#define WORLD_PILOTS 25 // the engine's PILOT_FIRE - PILOT_TOURNAMENT_FIRST
#define MAX_PILOTS 64

// The original pilots, who fight in the World Championship too (names compared on their letters).
static const char *originals[] = {"Crystal", "Steffan", "Milano",   "Christian", "Shirro",
                                  "Jean-Paul", "Ibrahim", "Angel", "Cossette",  "Raven"};

static bool same_letters(const char *a, const char *b) {
    for(;;) {
        while(*a && !isalpha((unsigned char)*a)) {
            a++;
        }
        while(*b && !isalpha((unsigned char)*b)) {
            b++;
        }
        if(!*a || !*b) {
            return !*a && !*b;
        }
        if(toupper((unsigned char)*a++) != toupper((unsigned char)*b++)) {
            return false;
        }
    }
}

static bool is_original(const char *name) {
    for(unsigned i = 0; i < sizeof(originals) / sizeof(originals[0]); i++) {
        if(same_letters(name, originals[i])) {
            return true;
        }
    }
    return false;
}

// The pilot loader asks the mod manager for extra photos; there are none here.
bool modmanager_get_player_pics(sd_pic_file *pic) {
    (void)pic;
    return false;
}

typedef struct {
    int w, h;
    unsigned char *px; // RGBA
} rgba;

static void rgba_create(rgba *img, int w, int h) {
    img->w = w;
    img->h = h;
    img->px = calloc(w * h, 4);
}

static int color_distance(vga_color a, int r, int g, int b) {
    int rm = (a.r + r) / 2;
    int dr = a.r - r, dg = a.g - g, db = a.b - b;
    return (((512 + rm) * dr * dr) >> 8) + 4 * dg * dg + (((767 - rm) * db * db) >> 8);
}

// The entries the original faces are drawn in, from 0xA1 (0xA0 has no darker copy).
static bool face_index(int k) {
    return k >= 0xA1 && k <= 0xEF && k != 0xA7 && k != 0xE7 && !(k >= 0xB8 && k <= 0xBF);
}

// The photo in its colors, cropped to 57x57: the top of the head, centered on it.
static void photo_face(rgba *out, const sd_pic_photo *photo, const vga_palette *scene) {
    sd_vga_image img;
    sd_sprite_vga_decode(&img, photo->sprite);
    // the head's middle: where the top half's pixels are
    long sum = 0, count = 0;
    for(unsigned y = 0; y < img.h / 2; y++) {
        for(unsigned x = 0; x < img.w; x++) {
            if(img.data[y * img.w + x]) {
                sum += x;
                count++;
            }
        }
    }
    int center = count ? (int)(sum / count) : (int)img.w / 2;
    int x0 = center - FACE_SIZE / 2;
    x0 = x0 + FACE_SIZE > (int)img.w ? (int)img.w - FACE_SIZE : x0;
    x0 = x0 < 0 ? 0 : x0;
    // narrower or shorter than the face: centered, at the bottom like the originals
    int dx = (int)img.w < FACE_SIZE ? (FACE_SIZE - (int)img.w) / 2 : 0;
    int dy = (int)img.h < FACE_SIZE ? FACE_SIZE - (int)img.h : 0;
    rgba_create(out, FACE_SIZE, FACE_SIZE);
    for(int y = 0; y < FACE_SIZE; y++) {
        for(int x = 0; x < FACE_SIZE; x++) {
            int sx = x - dx + (dx ? 0 : x0), sy = y - dy;
            if(sx < 0 || sy < 0 || sx >= (int)img.w || sy >= (int)img.h) {
                continue;
            }
            unsigned char q = img.data[sy * img.w + sx];
            if(!q) {
                continue;
            }
            vga_color c = q < 48 ? photo->pal.colors[q] : scene->colors[q];
            unsigned char *p = out->px + (y * FACE_SIZE + x) * 4;
            p[0] = c.r;
            p[1] = c.g;
            p[2] = c.b;
            p[3] = 255;
        }
    }
    sd_vga_image_free(&img);
}

// A secret opponent's face: Nicoli's, lit red on one side and blue on the other, repainted in the
// robot's colors (`ramp`, 16 shades from dark to light) by its brightness, stretched to the ramp.
static void recolor_face(rgba *out, const rgba *face, const vga_color *ramp) {
    int lo = 255 * 3, hi = 0;
    for(int i = 0; i < FACE_SIZE * FACE_SIZE; i++) {
        const unsigned char *p = face->px + i * 4;
        int l = p[0] + p[1] + p[2];
        if(p[3]) {
            lo = l < lo ? l : lo;
            hi = l > hi ? l : hi;
        }
    }
    rgba_create(out, FACE_SIZE, FACE_SIZE);
    for(int i = 0; i < FACE_SIZE * FACE_SIZE; i++) {
        const unsigned char *p = face->px + i * 4;
        if(!p[3]) {
            continue;
        }
        // lifted (the photos are dark), so that the lights reach the ramp's top: flames' yellow, ice's white
        float t = hi > lo ? (float)(p[0] + p[1] + p[2] - lo) / (hi - lo) : 0.5f;
        int k = 2 + (int)(13 * powf(t, 0.6f) + 0.5f);
        unsigned char *o = out->px + i * 4;
        o[0] = ramp[k].r;
        o[1] = ramp[k].g;
        o[2] = ramp[k].b;
        o[3] = 255;
    }
}

// The icon: the face scaled to 48x48 (area average), its rows 4..39 in the middle of 51.
static void face_icon(rgba *out, const rgba *face) {
    const int size = 48, top = 4;
    rgba_create(out, ICON_W, ICON_H);
    const float k = (float)FACE_SIZE / size;
    for(int y = 0; y < ICON_H; y++) {
        for(int x = 0; x < size; x++) {
            float r = 0, g = 0, b = 0, a = 0, n = 0;
            for(float sy = (y + top) * k; sy < (y + top + 1) * k; sy += 0.25f) {
                for(float sx = x * k; sx < (x + 1) * k; sx += 0.25f) {
                    const unsigned char *p = face->px + ((int)sy * FACE_SIZE + (int)sx) * 4;
                    n++;
                    if(p[3]) {
                        r += p[0];
                        g += p[1];
                        b += p[2];
                        a++;
                    }
                }
            }
            unsigned char *q = out->px + (y * ICON_W + x + (ICON_W - size) / 2) * 4;
            if(a * 2 >= n) {
                q[0] = r / a;
                q[1] = g / a;
                q[2] = b / a;
                q[3] = 255;
            }
        }
    }
}

// A sprite from the image in the palette's face colors; `background` fills what is
// transparent (0 keeps it transparent).
static void to_sprite(sd_sprite *s, const rgba *img, const vga_palette *pal, int background, vec2i pos) {
    sd_vga_image vga;
    sd_vga_image_create(&vga, img->w, img->h);
    for(int i = 0; i < img->w * img->h; i++) {
        const unsigned char *p = img->px + i * 4;
        int best = background, best_d = 1 << 30;
        if(p[3] >= 128) {
            for(int k = 1; k < 256; k++) {
                int d = face_index(k) ? color_distance(pal->colors[k], p[0], p[1], p[2]) : 1 << 30;
                if(d < best_d) {
                    best_d = d;
                    best = k;
                }
            }
        }
        vga.data[i] = (char)best;
    }
    sd_sprite_create(s);
    sd_sprite_vga_encode(s, &vga);
    s->pos = pos;
    sd_vga_image_free(&vga);
}

// Sets sprite `index`, appending copies of `filler` until the animation is that long.
static void put_sprite(sd_animation *ani, int index, sd_sprite *s, int filler) {
    while(sd_animation_get_sprite_count(ani) < index) {
        sd_sprite copy;
        sd_sprite_copy(&copy, sd_animation_get_sprite(ani, filler));
        copy.index = sd_animation_get_sprite_count(ani);
        sd_animation_push_sprite(ani, &copy);
        sd_sprite_free(&copy);
    }
    s->index = index;
    if(sd_animation_get_sprite_count(ani) > index) {
        sd_animation_set_sprite(ani, index, s); // rebuilt over a previous run
    } else {
        sd_animation_push_sprite(ani, s);
    }
}

static int add_faces(const char *dir, const char *bk_name, bool with_icons, const rgba faces[], int count) {
    char fn[1024];
    path p;
    sd_bk_file bk;
    sd_bk_create(&bk);
    snprintf(fn, sizeof(fn), "%s/%s", dir, bk_name);
    path_from_c(&p, fn);
    if(sd_bk_load(&bk, &p) != SD_SUCCESS) {
        fprintf(stderr, "cannot read %s\n", fn);
        return 1;
    }
    const vga_palette *pal = bk.palettes[0];
    sd_animation *face_ani = sd_bk_get_anim(&bk, FACES)->animation;
    vec2i face_pos = sd_animation_get_sprite(face_ani, 0)->pos;
    for(int n = 0; n < count; n++) {
        sd_sprite s;
        to_sprite(&s, &faces[n], pal, 0, face_pos);
        put_sprite(face_ani, FIRST_PILOT + n, &s, 0);
        sd_sprite_free(&s);
    }
    if(with_icons) {
        sd_animation *icon_ani = sd_bk_get_anim(&bk, ICONS)->animation;
        // the icons' black: the darkest face color
        int black = 0xA1, black_d = 1 << 30;
        for(int k = 1; k < 256; k++) {
            int d = face_index(k) ? color_distance(pal->colors[k], 0, 0, 0) : 1 << 30;
            if(d < black_d) {
                black_d = d;
                black = k;
            }
        }
        vec2i icon_pos = sd_animation_get_sprite(icon_ani, 0)->pos;
        for(int n = 0; n < count; n++) {
            rgba icon;
            face_icon(&icon, &faces[n]);
            sd_sprite s;
            to_sprite(&s, &icon, pal, black, icon_pos);
            put_sprite(icon_ani, FIRST_PILOT + n, &s, 0);
            sd_sprite_free(&s);
            free(icon.px);
        }
    }
    if(sd_bk_save(&bk, &p) != SD_SUCCESS) {
        fprintf(stderr, "cannot write %s\n", fn);
        return 1;
    }
    sd_bk_free(&bk);
    printf("wrote %s (pilots %d..%d)\n", fn, FIRST_PILOT, FIRST_PILOT + count - 1);
    return 0;
}

int main(int argc, char **argv) {
    if(argc != 2) {
        fprintf(stderr, "usage: %s <resources-dir>\n", argv[0]);
        return 1;
    }
    char fn[1024];
    path p;

    sd_tournament_file trn;
    sd_tournament_create(&trn);
    snprintf(fn, sizeof(fn), "%s/WORLD.TRN", argv[1]);
    path_from_c(&p, fn);
    if(sd_tournament_load(&trn, &p) != SD_SUCCESS) {
        fprintf(stderr, "cannot read %s\n", fn);
        return 1;
    }
    sd_pic_file pic;
    sd_pic_create(&pic);
    snprintf(fn, sizeof(fn), "%s/WORLD.PIC", argv[1]);
    path_from_c(&p, fn);
    if(sd_pic_load(&pic, &p) != SD_SUCCESS) {
        fprintf(stderr, "cannot read %s\n", fn);
        return 1;
    }
    // the photos' other colors: the VS screen's, where the tournament shows them
    sd_bk_file vs;
    sd_bk_create(&vs);
    snprintf(fn, sizeof(fn), "%s/VS.BK", argv[1]);
    path_from_c(&p, fn);
    if(sd_bk_load(&vs, &p) != SD_SUCCESS) {
        fprintf(stderr, "cannot read %s\n", fn);
        return 1;
    }

    rgba faces[MAX_PILOTS];
    int count = 0, nicoli = -1;
    for(int n = 0; n < trn.enemy_count && count < WORLD_PILOTS; n++) {
        if(is_original(str_c(&trn.enemies[n]->name))) {
            continue;
        }
        const sd_pic_photo *photo = sd_pic_get(&pic, trn.enemies[n]->photo_id);
        if(!photo) {
            fprintf(stderr, "no photo %d for %s\n", trn.enemies[n]->photo_id, str_c(&trn.enemies[n]->name));
            return 1;
        }
        if(same_letters(str_c(&trn.enemies[n]->name), "Nicoli")) {
            nicoli = count;
        }
        photo_face(&faces[count++], photo, vs.palettes[0]);
    }
    if(count != WORLD_PILOTS || nicoli < 0) {
        fprintf(stderr, "%d World Championship pilots, not %d, or no Nicoli\n", count, WORLD_PILOTS);
        return 1;
    }

    // Fire's and Ice's robots: the Fire Pit's fire (its second palette), and its third's ice sky
    sd_bk_file pit;
    sd_bk_create(&pit);
    snprintf(fn, sizeof(fn), "%s/ARENA3.BK", argv[1]);
    path_from_c(&p, fn);
    if(sd_bk_load(&pit, &p) != SD_SUCCESS || !pit.palettes[1] || !pit.palettes[2]) {
        fprintf(stderr, "cannot read %s's palettes\n", fn);
        return 1;
    }
    vga_color fire[16], ice[16];
    for(int k = 0; k < 16; k++) {
        fire[k] = pit.palettes[1]->colors[k];
        ice[k] = pit.palettes[2]->colors[48 + k];
    }
    recolor_face(&faces[count++], &faces[nicoli], fire);
    recolor_face(&faces[count++], &faces[nicoli], ice);
    sd_bk_free(&pit);
    sd_bk_free(&vs);

    int ret = add_faces(argv[1], "MELEE.BK", true, faces, count) || add_faces(argv[1], "VS.BK", false, faces, count);
    for(int n = 0; n < count; n++) {
        free(faces[n].px);
    }
    sd_pic_free(&pic);
    sd_tournament_free(&trn);
    return ret;
}
