#pragma once

#include <stdint.h>

enum EventKind {
    EventKindEncoder,
    EventKindButton
};

union EventData {
    int32_t encoder_delta;
    int button_num;
};
