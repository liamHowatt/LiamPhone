#pragma once

#include <stdint.h>
#include "async.h"

#define HOST_CODE_DISPLAY 1

typedef struct {
    uint8_t code;
    uint8_t *bytes;
    int len;
    uint8_t *ret;
} host_transmit_data;

void host_init();
ListNode *host_wait_for_ms_or_event(int wait_time);

ASYNC_DECLARE_FUNCTION(host_transmit, host_transmit_data);
