#include "screen_machine.h"
#include "list.h"
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>

struct ScreenInstance {
    void *data;
    const struct ScreenInterface *interface;
};

static ListNode *screen_stack = NULL;

static void draw_and_push(const struct ScreenInterface *interface, void *data) {
    interface->draw(data);

    struct ScreenInstance *instance = malloc(sizeof(struct ScreenInstance));
    instance->data = data;
    instance->interface = interface;
    list_push(&screen_stack, instance);
}

void screen_machine_init(const struct ScreenInterface *root_interface, void *root_data) {
    draw_and_push(root_interface, root_data);
}

void screen_machine_send_event_top(enum EventKind event_kind, union EventData event_data) {
    struct ScreenInstance *top_screen = screen_stack->value;
    struct ScreenWantsTo screen_wants_to = top_screen->interface->on_event(top_screen->data, event_kind, event_data);
    if (screen_wants_to.action == ScreenWantsToDraw) {
        top_screen->interface->draw(top_screen->data);
    }
    else if (screen_wants_to.action == ScreenWantsToClose) {
        top_screen->interface->destroy(top_screen->data);
        free(top_screen);
        list_discard(&screen_stack);
        top_screen = screen_stack->value;
        top_screen->interface->draw(top_screen->data);
    }
    else if (screen_wants_to.action == ScreenWantsToOpenScreen) {
        draw_and_push(screen_wants_to.data.open_screen.interface, screen_wants_to.data.open_screen.data);
    }
}

void screen_machine_send_event_all(enum EventKind event_kind, union EventData event_data) {
    ListNode *screen_node = screen_stack;
    while (screen_node) {
        struct ScreenInstance *screen = screen_node->value;
        // struct ScreenWantsTo screen_wants_to = screen->interface->on_event(screen->data, event_kind, event_data);
        screen->interface->on_event(screen->data, event_kind, event_data);
        screen_node = screen_node->next;
    }
}

void screen_machine_draw_me_if_top(void *data) {
    struct ScreenInstance *top_screen = screen_stack->value;
    if (data == top_screen->data) {
        top_screen->interface->draw(data);
    }
}
