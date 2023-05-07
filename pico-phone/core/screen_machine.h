#pragma once

#include "screen.h"
#include "events.h"

void screen_machine_init(const struct ScreenInterface *root_interface, void *root_data);
void screen_machine_send_event_top(enum EventKind event_kind, union EventData event_data);
void screen_machine_send_event_all(enum EventKind event_kind, union EventData event_data);
void screen_machine_draw_me_if_top(void *data);
