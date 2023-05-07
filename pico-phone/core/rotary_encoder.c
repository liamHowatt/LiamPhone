#include "rotary_encoder_impl.h"
#include "async.h"
#include "stdbool.h"
#include "screen_machine.h"
#include <stdio.h>

static int32_t value;
static bool first_time = true;


static void cb(int32_t reading);

static ASYNC_DEFINE_FUNCTION(encoder_cooldown, void)
    ASYNC_SLEEP(8);
    rotary_encoder_read(cb);
ASYNC_DEFINE_FUNCTION_END

static void cb(int32_t reading) {
    if (first_time) {
        first_time = false;
    } else {
        if (reading != value) {
            int32_t delta = reading - value;
            printf("delta: %d\n", (int) delta);
            screen_machine_send_event_top(EventKindEncoder, (union EventData) { .encoder_delta=delta });
        }
    }
    value = reading;
    async_spawn(encoder_cooldown, NULL);
}

void rotary_encoder_init() {
    async_spawn(encoder_cooldown, NULL);
}
