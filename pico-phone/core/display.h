#pragma once

#include <stdint.h>
#include <stdbool.h>

#define FB_LEN 12482

extern uint8_t display_fb[FB_LEN];

void display_init();
void display_fill(uint8_t val);
void display_draw_glyph(const uint8_t *glyph, int x, int y, bool invert);
void display_draw_ascii_char(char c, int x, int y, bool invert);
void display_draw_text(const char *src, int len, int x, int y, bool invert);
