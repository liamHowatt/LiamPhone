#pragma once

#include "async_impl_types.h"
#include <stdlib.h>
#include "list.h"
#include <stdbool.h>

struct async_lock_t {
    bool held;
    int waiting_stacks_len;
    ListNode **waiting_stacks;
};

union async_return_ {
    // code0, nothing, done
    struct { int line; async_delta_time sleep_dur_ms; } code1_sleep;
    struct { int line; int (*func) (int, void **, union async_return_ *); void *data; } code2_await;
    struct { int line; } code3_detach;
    struct { int line; struct async_lock_t *lock; } code4_lock_acquire;
};

void async_spawn(int (*func) (int, void **, union async_return_ *), void *data);
void async_loop();

#define ASYNC_DECLARE_FUNCTION(fn_name, data_type) int fn_name(int async_arg_jumpto, void **async_arg_data, union async_return_ *async_arg_ret)
#define ASYNC_DEFINE_FUNCTION(fn_name, data_type) ASYNC_DECLARE_FUNCTION(fn_name, data_type) { \
    data_type *data = *async_arg_data; \
    (void)data; \
    switch (async_arg_jumpto) { case 0:;
#define ASYNC_DEFINE_FUNCTION_END }return 0;}
#define ASYNC_ALLOCATE_DATA(data_type) do {data = *((data_type **) async_arg_data) = malloc(sizeof(data_type));} while (0)

#define ASYNC_RETURN              do {return 0;} while (0)
#define ASYNC_SLEEP(ms)           do {async_arg_ret->code1_sleep.line = __LINE__; async_arg_ret->code1_sleep.sleep_dur_ms = ms; return 1; case __LINE__:;} while (0)
#define ASYNC_AWAIT(func_, data_) do {async_arg_ret->code2_await.line = __LINE__; async_arg_ret->code2_await.func = func_; async_arg_ret->code2_await.data = data_; return 2; case __LINE__:;} while (0)

#define ASYNC_LOCK_INITIALIZE_NOT_HELD (struct async_lock_t) { .held=false, .waiting_stacks_len=0, .waiting_stacks=NULL };
#define ASYNC_LOCK_INITIALIZE_HELD (struct async_lock_t) { .held=true, .waiting_stacks_len=0, .waiting_stacks=NULL };
#define ASYNC_LOCK_ACQUIRE(lock_ptr_) do {async_arg_ret->code4_lock_acquire.line = __LINE__; async_arg_ret->code4_lock_acquire.lock = lock_ptr_; return 4; case __LINE__:;} while (0)
bool async_lock_try_acquire(struct async_lock_t *lock);
void async_lock_release(struct async_lock_t *lock);
