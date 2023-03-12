#pragma once

#include <stdint.h>

#define FB_LEN 12482

extern uint8_t display_fbs[2][FB_LEN];
extern int display_usr_fb_i;

void display_init();
void display_fill(uint8_t val);
void display_draw_glyph(const uint8_t *glyph, int x, int y);
