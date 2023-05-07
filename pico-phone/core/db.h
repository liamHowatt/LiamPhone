#pragma once

#include <stdbool.h>
#include <stdint.h>

void db_init();

extern struct async_lock_t db_lock;

extern int16_t db_text_index_len;

struct db_lookup_res {
    bool found_match;
    union {
        char *found_match_result;
        int16_t no_match_closest_index;
    };
};

struct db_lookup_res db_contacts_lookup_name_by_number(const char *number);

struct db_text_index_t {
    char *number;
    uint8_t unreads;
    char timestamp[14];
    char *msg_preview;
};
struct db_text_index_t *db_get_convo_from_index(int16_t i);
