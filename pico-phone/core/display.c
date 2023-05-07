#include <string.h>
#include "display.h"
#include "display_driver.h"
#include "glyphs.h"
#include <stdio.h>

uint8_t display_fb[FB_LEN];

static uint8_t reverse_uint8_bits(uint8_t x) {
    uint8_t y = 0;
    int i = 7;
    while (x) {
        y |= (x & 1) << i;
        i--;
        x >>= 1;
    }
    return y;
}

void display_init() {
    memset(display_fb, 0, FB_LEN);
    int i = 1;
    for (int row=1; row<241; row++) {
        display_fb[i] = reverse_uint8_bits(row);
        i += 52;
    }
    display_driver_init();
}

void display_fill(uint8_t val) {
    uint8_t *p = display_fb + 2;
    for (int i=0; i<240; i++) {
        memset(p, val, 50);
        p += 52;
    }
}

void display_draw_glyph(const uint8_t *glyph, int x, int y, bool invert) {
    if (x < 0 || x > 49 || y < 0 || y > 29) {
        return;
    }
    uint8_t *p = display_fb + ((y * (52 * 8)) + x + 2);
    for (int i=0; i<8; i++) {
        uint8_t row;
        if (invert) {
            row = ~(glyph[i]);
        } else {
            row = glyph[i];
        }
        *p = row;
        p += 52;
    }
}

void display_draw_ascii_char(char c, int x, int y, bool invert) {
    if (c >= ' ' && c <= '~') {
        display_draw_glyph(glyph_ascii[c - ' '], x, y, invert);
    } else {
        display_draw_glyph(glyph_ascii_invalid, x, y, invert);
    }
}

void display_draw_text(const char *src, int len, int x, int y, bool invert) {
    while (len-- && *src) {
        display_draw_ascii_char(*(src++), x++, y, invert);
    }
}
