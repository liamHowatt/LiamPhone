#include "display_driver.h"
#include "host_bridge.h"
#include "display.h"
#include <stdio.h>


static ASYNC_DEFINE_FUNCTION(display_driver_task, host_transmit_data)
    while (1) {
        // puts("sending frame...");
        ASYNC_AWAIT(host_transmit, data);
        ASYNC_SLEEP(50);
    }
ASYNC_DEFINE_FUNCTION_END

void display_driver_init() {
    static host_transmit_data task_data;
    task_data.code = HOST_CODE_DISPLAY;
    task_data.bytes = display_fb;
    task_data.len = FB_LEN;
    async_spawn(display_driver_task, &task_data);
}
