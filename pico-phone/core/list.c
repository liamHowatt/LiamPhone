#include "list.h"
#include <stdlib.h>

void list_push(ListNode **end, void *value) {
    ListNode *new_node = malloc(sizeof(ListNode));
    new_node->value = value;
    new_node->next = *end;
    *end = new_node;
}

void list_pop(ListNode **end, void **popped) {
    ListNode *node_being_popped = *end;
    if (node_being_popped == NULL) {
        *popped = NULL;
        return;
    }
    *end = node_being_popped->next;
    *popped = node_being_popped->value;
    free(node_being_popped);
}

void list_discard(ListNode **end) {
    ListNode *node_being_discarded = *end;
    *end = node_being_discarded->next;
    free(node_being_discarded);
}
