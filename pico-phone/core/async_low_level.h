#pragma once
#include "async.h"

void *async_get_task_stack();

#define ASYNC_DETACH() do {async_arg_ret->code3_detach.line = __LINE__; return 3; case __LINE__:;} while (0)
