#include "async.h"
#include "button_impl.h"
#include "screen_machine.h"

static ASYNC_DEFINE_FUNCTION(button_task, void)
    static int button_value;
    while (1) {
        ASYNC_AWAIT(button_read, &button_value);
        if (button_value >= 0) {
            screen_machine_send_event_top(EventKindButton, (union EventData) { .button_num=button_value });
        }
    }
ASYNC_DEFINE_FUNCTION_END

void button_init() {
    async_spawn(button_task, NULL);
}
