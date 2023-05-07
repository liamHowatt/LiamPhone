#include "core.h"
#include "host_bridge.h"

int main() {
    host_init();
    core();
}

// #include <stdio.h>
// #include "async.h"

// static struct async_lock_t lock = ASYNC_LOCK_INITIALIZER;

// static ASYNC_DEFINE_FUNCTION(mini_task, void)
//     ASYNC_LOCK_ACQUIRE(&lock);
//     ASYNC_SLEEP(1000);
//     ASYNC_LOCK_RELEASE(&lock);
//     puts("OH YEAH");
// ASYNC_DEFINE_FUNCTION_END

// static ASYNC_DEFINE_FUNCTION(wait2, void)
//     puts("wait 2 start");
//     ASYNC_SLEEP(1500);
//     puts("wait 2 done");
// ASYNC_DEFINE_FUNCTION_END

// int main() {
//     host_init();
//     async_spawn(mini_task, NULL);
//     async_spawn(mini_task, NULL);
//     async_spawn(mini_task, NULL);
//     async_spawn(wait2, NULL);
//     async_spawn(mini_task, NULL);
//     async_spawn(mini_task, NULL);
//     async_spawn(mini_task, NULL);
//     async_loop();
// }
