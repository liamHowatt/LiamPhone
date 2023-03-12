#pragma once

#include "async_impl_types.h"
#include <stdlib.h>

typedef struct list_node_t {
    void *value;
    struct list_node_t *next;
} ListNode;

union async_return_ {
    // code0, nothing, done
    struct { int line; async_delta_time sleep_dur_ms; } code1_sleep;
    struct { int line; int (*func) (int, void **, union async_return_ *); void *data; } code2_await;
    struct { int line; } code3_detach;
};

void async_spawn(int (*func) (int, void **, union async_return_ *), void *data);
void async_loop();

#define ASYNC_DECLARE_FUNCTION(fn_name, data_type) int fn_name(int async_arg_jumpto, void **async_arg_data, union async_return_ *async_arg_ret)
#define ASYNC_DEFINE_FUNCTION(fn_name, data_type) ASYNC_DECLARE_FUNCTION(fn_name, data_type) { \
    data_type *data = *async_arg_data; \
    switch (async_arg_jumpto) { case 0:;
#define ASYNC_DEFINE_FUNCTION_END }return 0;}
#define ASYNC_ALLOCATE_DATA(data_type) {data = *((data_type **) async_arg_data) = malloc(sizeof(data_type));} while (0)

#define ASYNC_RETURN()            {return 0;} while (0)
#define ASYNC_SLEEP(ms)           {async_arg_ret->code1_sleep.line = __LINE__; async_arg_ret->code1_sleep.sleep_dur_ms = ms; return 1; case __LINE__:;} while (0)
#define ASYNC_AWAIT(func_, data_) {async_arg_ret->code2_await.line = __LINE__; async_arg_ret->code2_await.func = func_; async_arg_ret->code2_await.data = data_; return 2; case __LINE__:;} while (0)
