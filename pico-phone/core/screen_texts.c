#include "screen_texts.h"
#include "button.h"
#include <stddef.h>
#include "display.h"
#include "async.h"
#include <stdbool.h>
#include <stdlib.h>
#include "screen_machine.h"
#include <stdint.h>
#include "db.h"
#include <string.h>
#include "glyphs.h"

static bool loader_run = false;
static bool has_db_lock = false;

static uint8_t uid_pointee;
static void *uid = &uid_pointee;

static int selected = 0;

static ASYNC_DEFINE_FUNCTION(loader, void)
    if (!loader_run) {
        ASYNC_RETURN;
    }
    ASYNC_LOCK_ACQUIRE(&db_lock);
    has_db_lock = true;
    if (loader_run) {
        screen_machine_draw_me_if_top(uid);
        loader_run = false;
    }
    has_db_lock = false;
    async_lock_release(&db_lock);
ASYNC_DEFINE_FUNCTION_END

static void *init(void *initializer) {
    return uid;
}

static void draw_loading() {
    display_draw_text("loading...", -1, 0, 0, false);
}

static const char timestamp_delimiters[] = {'/', '/', '-', ':', ':', '+'};
static void draw_actual() {
    for (int i=selected*2; i<=selected*2+1; i++) {
        for (int j=0; j<50; j++) {
            display_draw_glyph(glyph_white, j, i, false);
        }
    }
    int limit = db_text_index_len > 15 ? 15 : db_text_index_len;
    for (int i=0; i<limit; i++) {
        bool is_sel = i == selected;
        struct db_text_index_t *entry = db_get_convo_from_index(i);
        struct db_lookup_res name_lookup = db_contacts_lookup_name_by_number(entry->number);
        char *name_text = name_lookup.found_match ? name_lookup.found_match_result : entry->number;
        int unreads_width;
        if (entry->unreads > 99) {
            unreads_width = 3;
        } else if (entry->unreads > 9) {
            unreads_width = 2;
        } else {
            unreads_width = 1;
        }
        int name_text_room = (50 - 14 - 6 - 1 - 1) - unreads_width;
        int name_text_true_length = strlen(name_text);
        int unreads_cursor;
        if (name_text_room >= name_text_true_length) {
            display_draw_text(name_text, -1, 0, i * 2, false ^ is_sel);
            unreads_cursor = name_text_true_length + 1;
        } else {
            display_draw_text(name_text, name_text_room - 3, 0, i * 2, false ^ is_sel);
            display_draw_text("...", -1, name_text_room - 3, i * 2, false ^ is_sel);
            unreads_cursor = name_text_room + 1;
        }

        if (entry->unreads) {
            if (unreads_width == 3) {
                display_draw_text("99+", -1, unreads_cursor, i * 2, true ^ is_sel);
            } else {
                if (unreads_width == 2) {
                    display_draw_ascii_char((entry->unreads / 10) + '0', unreads_cursor++, i * 2, true ^ is_sel);
                }
                display_draw_ascii_char((entry->unreads % 10) + '0', unreads_cursor, i * 2, true ^ is_sel);
            }
        }

        int timestamp_cursor_entry = 0;
        int timestamp_cursor_screen = (50 - 14 - 6);
        for (int j=0; j<6; j++) {
            display_draw_text(&entry->timestamp[timestamp_cursor_entry], 2, timestamp_cursor_screen, i * 2, false ^ is_sel);
            timestamp_cursor_entry += 2;
            timestamp_cursor_screen += 2;
            display_draw_ascii_char(timestamp_delimiters[j], timestamp_cursor_screen, i * 2, false ^ is_sel);
            timestamp_cursor_screen += 1;
        }
        display_draw_text(&entry->timestamp[timestamp_cursor_entry], 2, timestamp_cursor_screen, i * 2, false ^ is_sel);

        display_draw_text(entry->msg_preview, -1, 0, (i * 2) + 1, false ^ is_sel);
    }
}

static void draw(void *self) {
    display_fill(0);
    if (loader_run) {
        if (has_db_lock) {
            draw_actual();
        } else {
            draw_loading();
        }
    }
    else {
        if (async_lock_try_acquire(&db_lock)) {
            draw_actual();
            async_lock_release(&db_lock);
        }
        else {
            draw_loading();
            loader_run = true;
            async_spawn(loader, NULL);
        }
    }
}

static struct ScreenWantsTo on_event(void *self, enum EventKind event_kind, union EventData event_data) {
    if (event_kind == EventKindButton) {
        if (event_data.button_num == BUTTON_B) {
            return (struct ScreenWantsTo) {.action=ScreenWantsToClose};
        }
    }
    return (struct ScreenWantsTo) {.action=ScreenWantsToNothing};
}

static void destroy(void *self) {
    loader_run = false;
    selected = 0;
}

const struct ScreenInterface screen_texts_interface = {
    .init=init,
    .draw=draw,
    .on_event=on_event,
    .destroy=destroy
};
