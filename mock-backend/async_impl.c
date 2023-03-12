#include "async_impl.h"
#include <time.h>
#include <assert.h>
#include "host_bridge.h"

static void millisleep(long msec)
{
    struct timespec ts;
    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;
    assert(!nanosleep(&ts, NULL));
}

static long millitime() {
    struct timespec ts;
    assert(!clock_gettime(CLOCK_MONOTONIC_RAW, &ts));
    long ret = (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
    assert(ret >= 0);
    return ret;
}

async_abs_time async_get_time_after(async_delta_time delta) {
    return millitime() + delta;
}

async_abs_time async_get_time_now() {
    return millitime();
}

async_diff_time async_abs_time_diff(async_abs_time from, async_abs_time to) {
    return to - from;
}

async_abs_time async_null_time() {
    return -1;
}

bool async_time_is_null(async_abs_time time) {
    return time == -1;
}


ListNode *async_wait_until_time_or_event(async_abs_time abs) {
    long wait_time = abs - millitime();
    if (wait_time < 0) {
        wait_time = 0;
    }
    return host_wait_for_ms_or_event(wait_time);
}

ListNode *async_wait_forever_for_event() {
    return host_wait_for_ms_or_event(-1);
}


