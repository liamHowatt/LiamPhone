#include <stdlib.h>
#include "async.h"
#include "async_impl.h"
#include <string.h>
// #include <stdio.h>

typedef struct {
    int line;
    int (*func) (int, void **, union async_return_ *);
    void *data;
} TaskStackValue;
static TaskStackValue *new_task_stack_value(
    int line,
    int (*func) (int, void **, union async_return_ *),
    void *data
) {
    TaskStackValue *new = malloc(sizeof(TaskStackValue));
    new->line = line;
    new->func = func;
    new->data = data;
    return new;
}

typedef struct {
    async_abs_time wake_time;
    ListNode *task_stack;
} ScheduleValue;
static ScheduleValue *new_schedule_value(
    async_abs_time wake_time,
    ListNode *task_stack
) {
    ScheduleValue *new = malloc(sizeof(ScheduleValue));
    new->wake_time = wake_time;
    new->task_stack = task_stack;
    return new;
}

static ListNode *schedule = NULL;

// void print_schedule(int line) {
//     printf("\nprint_schedule called at line %d\n", line);
//     ListNode *schedule_node = schedule;
//     while (schedule_node != NULL) {
//         ScheduleValue *sched_val = schedule_node->value;
//         printf("time: %ld\n", sched_val->wake_time);
//         ListNode *stack = sched_val->task_stack;
//         while (stack) {
//             TaskStackValue *stack_val = stack->value;
//             struct talker_data_t *data = stack_val->data;
//             if (data)
//                 printf("    line: %d, id: %d, n: %d, sleep dur: %ld\n", stack_val->line, data->id, data->n, data->sleep_dur);
//             stack = stack->next;
//         }
//         schedule_node = schedule_node->next;
//     }
//     puts("");
//     getchar();
// }
// #define print_schedule(x)

static void schedule_a_value(ScheduleValue *value) {
    ListNode *last = NULL;
    ListNode *node_i = schedule;
    bool node_i_is_schedule = true;
    if (!async_time_is_null(value->wake_time)){
        while (
            node_i != NULL && (
                async_time_is_null(((ScheduleValue *) node_i->value)->wake_time)
                || async_abs_time_diff(((ScheduleValue *) node_i->value)->wake_time, value->wake_time) >= 0
            )
        ) {
            node_i_is_schedule = false;
            last = node_i;
            node_i = node_i->next;
        }
    }
    list_push(&node_i, value);
    if (last != NULL) {
        last->next = node_i;
    }
    if (node_i_is_schedule) {
        schedule = node_i;
    }
}

static ListNode *task_stack;

// PUBLIC API

void async_spawn(int (*func) (int, void **, union async_return_ *), void *data) {
    TaskStackValue *task_stack_value = new_task_stack_value(0, func, data);
    ListNode *task_stack = NULL;
    list_push(&task_stack, task_stack_value);

    schedule_a_value(new_schedule_value(async_null_time(), task_stack));
}

void async_loop() {
    ListNode *shortcut_task_stack = NULL;
    while (1) {
        if (shortcut_task_stack) {
            task_stack = shortcut_task_stack;
            shortcut_task_stack = NULL;
        }
        else {
            ScheduleValue *schedule_value;
            if (schedule == NULL) {
                schedule_value = NULL;
            } else {
                schedule_value = schedule->value;
            }

            ListNode *evented_task_stack;
            if (schedule_value != NULL) {
                if (!async_time_is_null(schedule_value->wake_time)) {
                    evented_task_stack = async_wait_until_time_or_event(schedule_value->wake_time);
                    if (evented_task_stack == NULL) {
                        task_stack = schedule_value->task_stack;
                        free(schedule_value);
                        list_discard(&schedule);
                    }
                    else {
                        task_stack = evented_task_stack;
                    }
                }
                else {
                    task_stack = schedule_value->task_stack;
                    free(schedule_value);
                    list_discard(&schedule);
                }
            } else {
                evented_task_stack = async_wait_forever_for_event();
                if (evented_task_stack == NULL) {
                    return;
                }
                task_stack = evented_task_stack;
            }


        }

        TaskStackValue *task_stack_value = task_stack->value;

        union async_return_ ret;
        int ret_code = task_stack_value->func(
            task_stack_value->line,
            &task_stack_value->data,
            &ret
        );

        if (ret_code == 0) { // return
            free(task_stack_value);
            list_discard(&task_stack);
            if (task_stack != NULL) {
                shortcut_task_stack = task_stack;
            }
        }
        else if (ret_code == 1) { // sleep
            task_stack_value->line = ret.code1_sleep.line;
            schedule_a_value(new_schedule_value(async_get_time_after(ret.code1_sleep.sleep_dur_ms), task_stack));
        }
        else if (ret_code == 2) { // await
            task_stack_value->line = ret.code2_await.line;
            list_push(&task_stack, new_task_stack_value(0, ret.code2_await.func, ret.code2_await.data));
            shortcut_task_stack = task_stack;
        }
        else if (ret_code == 3) { // detach
            task_stack_value->line = ret.code3_detach.line;
        }
        else if (ret_code == 4) { // lock acquire
            task_stack_value->line = ret.code4_lock_acquire.line;
            if (async_lock_try_acquire(ret.code4_lock_acquire.lock)) {
                shortcut_task_stack = task_stack;
            }
            else {
                struct async_lock_t *lock = ret.code4_lock_acquire.lock;
                lock->waiting_stacks = realloc(lock->waiting_stacks, (lock->waiting_stacks_len + 1) * sizeof(ListNode *));
                memmove(lock->waiting_stacks + 1, lock->waiting_stacks, lock->waiting_stacks_len * sizeof(ListNode *));
                lock->waiting_stacks[0] = task_stack;
                lock->waiting_stacks_len += 1;
            }
        }
    }
}

void *async_get_task_stack() {
    return task_stack;
}

bool async_lock_try_acquire(struct async_lock_t *lock) {
    if (lock->held) {
        return false;
    }
    lock->held = true;
    return true;
}

void async_lock_release(struct async_lock_t *lock) {
    if (lock->waiting_stacks_len == 0) {
        lock->held = false;
    }
    else {
        lock->waiting_stacks_len -= 1;
        schedule_a_value(new_schedule_value(async_null_time(), lock->waiting_stacks[lock->waiting_stacks_len]));
        if (lock->waiting_stacks_len == 0) {
            free(lock->waiting_stacks);
            lock->waiting_stacks = NULL;
        }
        else {
            lock->waiting_stacks = realloc(lock->waiting_stacks, lock->waiting_stacks_len * sizeof(ListNode *));
        }
    }
}
