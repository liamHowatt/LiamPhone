#pragma once

typedef struct list_node_t {
    void *value;
    struct list_node_t *next;
} ListNode;

void list_push(ListNode **end, void *value);
void list_pop(ListNode **end, void **popped);
void list_discard(ListNode **end);
