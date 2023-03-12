#include "display_driver.h"
#include "host_bridge.h"
#include "display.h"
#include <stdio.h>


ASYNC_DEFINE_FUNCTION(display_driver_task, host_transmit_data)
    ASYNC_ALLOCATE_DATA(host_transmit_data);
    data->code = HOST_CODE_DISPLAY;
    data->bytes = display_fbs[display_usr_fb_i];
    data->len = FB_LEN;
    while (1) {
        // puts("sending frame...");
        ASYNC_AWAIT(host_transmit, data);
    }
    // free(data);
ASYNC_DEFINE_FUNCTION_END
