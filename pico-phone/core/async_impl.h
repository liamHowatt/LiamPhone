#pragma once

#include "async.h"
#include <stdbool.h>

async_abs_time async_get_time_after(async_delta_time delta);
async_abs_time async_get_time_now();
async_diff_time async_abs_time_diff(async_abs_time from, async_abs_time to);
async_abs_time async_null_time();
bool async_time_is_null(async_abs_time time);

ListNode *async_wait_until_time_or_event(async_abs_time abs);
ListNode *async_wait_forever_for_event();
