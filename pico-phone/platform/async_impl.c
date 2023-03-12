#include "async_impl.h"

async_abs_time async_get_time_after(async_delta_time delta) {
    return make_timeout_time_ms(delta);
}

async_abs_time async_get_time_now() {
    return get_absolute_time();
}

async_diff_time async_abs_time_diff(async_abs_time from, async_abs_time to) {
    return absolute_time_diff_us(from, to);
}

async_abs_time async_null_time() {
    return nil_time;
}

bool async_time_is_null(async_abs_time time) {
    return is_nil_time(time);
}


ListNode *async_wait_until_time_or_event(async_abs_time abs) {
    sleep_until(abs);
    return NULL;
}

ListNode *async_wait_forever_for_event() {
    return NULL;
}
