#include <string.h>
#include "display.h"
#include "display_driver.h"

uint8_t display_fbs[2][FB_LEN];
int display_usr_fb_i;

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
    memset(display_fbs[0], 0, FB_LEN);
    int i = 1;
    for (int row=1; row<241; row++) {
        display_fbs[0][i] = reverse_uint8_bits(row);
        i += 52;
    }
    memcpy(display_fbs[1], display_fbs[0], FB_LEN);
    display_usr_fb_i = 0;
    async_spawn(display_driver_task, NULL);
}

void display_fill(uint8_t val) {
    uint8_t *p = display_fbs[display_usr_fb_i] + 2;
    for (int i=0; i<240; i++) {
        memset(p, val, 50);
        p += 52;
    }
}

void display_draw_glyph(const uint8_t *glyph, int x, int y) {
    if (x < 0 || x > 49 || y < 0 || y > 29) {
        return;
    }
    uint8_t *p = display_fbs[display_usr_fb_i] + ((y * (52 * 8)) + x + 2);
    for (int i=0; i<8; i++) {
        *p = glyph[i];
        p += 52;
    }
}
