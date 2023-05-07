#include <stdio.h>
#include <stdlib.h>
#include "async.h"
#include "onboard_led.h"
#include "display.h"
#include "glyphs.h"
#include "screen_home.h"
#include "screen_machine.h"
#include "rotary_encoder.h"
#include "fs.h"
#include "button.h"
#include "db.h"

// static int x = 0, y = 0, ascii = 0;
// static ASYNC_DEFINE_FUNCTION(gui, void)
//     display_fill(255);
//     while (1) {
//         display_draw_glyph(glyph_ascii[ascii], x, y, true);
//         ascii++;
//         if (ascii > 94) {
//             ascii = 0;
//         }
//         x++;
//         if (x > 49) {
//             x = 0;
//             y++;
//             if (y > 29) {
//                 y = 0;
//                 display_fill(255);
//             }
//         }
//         ASYNC_SLEEP(1);
//     }
// ASYNC_DEFINE_FUNCTION_END

// static ASYNC_DEFINE_FUNCTION(tmp, void)
//     static struct db_contacts_lookup_name_from_number_data lookup_data;
//     lookup_data.number = "939";
//     ASYNC_AWAIT(db_contacts_lookup_name_from_number, &lookup_data);
//     if (lookup_data.ret_found_match) {
//         printf("name: %s\n", lookup_data.ret_name);
//         free(lookup_data.ret_name);
//     } else {
//         puts("nahhh");
//     }
// ASYNC_DEFINE_FUNCTION_END

void core() {
    printf("begin\n");
    display_init();
    // async_spawn(gui, NULL);
    fs_init();
    db_init();
    screen_machine_init(&screen_home_interface, screen_home_interface.init(NULL));
    rotary_encoder_init();
    button_init();
    // async_spawn(tmp, NULL);
    async_loop();
}
