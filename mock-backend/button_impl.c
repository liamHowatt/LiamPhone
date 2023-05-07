#include "button_impl.h"
#include "host_bridge.h"

ASYNC_DEFINE_FUNCTION(button_read, int)
    ASYNC_SLEEP(50);
    static host_transmit_data host_data = {.code=HOST_CODE_BUTTON, .bytes=NULL, .len=0};
    ASYNC_AWAIT(host_transmit, &host_data);
    int8_t btn = *((int8_t *) host_data.ret);
    free(host_data.ret);
    *data = btn;
ASYNC_DEFINE_FUNCTION_END

