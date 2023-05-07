#pragma once

#include "events.h"

struct ScreenWantsTo {
    enum {
        ScreenWantsToNothing,
        ScreenWantsToDraw,
        ScreenWantsToClose,
        ScreenWantsToOpenScreen
    } action;
    union {
        struct {const struct ScreenInterface *interface; void *data;} open_screen;
    } data;
};

struct ScreenInterface {
    void *(*init)(void *initializer);
    void (*draw)(void *self);
    struct ScreenWantsTo (*on_event)(void *self, enum EventKind event_kind, union EventData event_data);
    void (*destroy)(void *self);
};

