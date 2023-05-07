#include "screen_home.h"
#include "display.h"
#include "glyphs.h"
#include "stddef.h"
#include "screen_texts.h"
#include "button.h"

static int selected;
static const int num_entries = 6;
static const char *entries[] = {
    "phone",
    "texts",
    "contacts",
    "telegram",
    "settings",
    "debug\t"
};

static void *init(void *initializer) {
    selected = 0;
    return NULL;
}

static void draw(void *self) {
    display_fill(0);
    for (int i=0; i<num_entries; i++) {
        int c = 0;
        bool highlighted = i == selected;
        char ch;
        while ((ch = entries[i][c])) {
            display_draw_ascii_char(ch, c, i, highlighted);
            c += 1;
        }
        if (highlighted) {
            while (c < 50) {
                display_draw_glyph(glyph_white, c, i, false);
                c += 1;
            }
        }
    }
}

// by Jason Turner
// https://www.youtube.com/watch?v=xVNYurap-lk
// https://compiler-explorer.com/z/1dMMsMz7M
static int floored_modulo(int dividend, int divisor) {
    return ((dividend % divisor) + divisor) % divisor;
}

static struct ScreenWantsTo on_event(void *self, enum EventKind event_kind, union EventData event_data) {
    if (event_kind == EventKindEncoder) {
        selected = floored_modulo(selected + event_data.encoder_delta, num_entries);
        return (struct ScreenWantsTo) {.action=ScreenWantsToDraw};
    }
    else if (event_kind == EventKindButton) {
        if (event_data.button_num == BUTTON_A) {
            if (selected == 1) { // TODO identify texts option better
                return (struct ScreenWantsTo) {
                    .action=ScreenWantsToOpenScreen,
                    .data.open_screen={.interface=&screen_texts_interface, .data=screen_texts_interface.init(NULL)}
                };
            }
        }
    }
    return (struct ScreenWantsTo) {.action=ScreenWantsToNothing};
}

static void destroy(void *self) {
}

const struct ScreenInterface screen_home_interface = {
    .init=init,
    .draw=draw,
    .on_event=on_event,
    .destroy=destroy
};
