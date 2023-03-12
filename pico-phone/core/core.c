#include <stdio.h>
#include <stdlib.h>
#include "async.h"
#include "onboard_led.h"
#include "display.h"
#include "glyphs.h"

static int x = 0, y = 0;
static ASYNC_DEFINE_FUNCTION(glypher, void)
    while (1) {
        display_fill(255);
        display_draw_glyph(glyph_black, x, y);
        x++;
        if (x > 49) {
            x = 0;
            y++;
            if (y > 29) {
                y = 0;
            }
        }
        ASYNC_SLEEP(100);
    }
ASYNC_DEFINE_FUNCTION_END

void core() {
    display_init();
    async_spawn(glypher, NULL);
    async_loop();
}
