#include "async.h"
#include <stdint.h>
#include <string.h>
#include "db.h"
#include "fs.h"
#include <stdio.h>

#if __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
#error designed to work with little endian only
#endif

struct async_lock_t db_lock = ASYNC_LOCK_INITIALIZE_HELD;

struct contact_t {
    char *name;
    char *number;
};
static struct contact_t *contacts = NULL;
static int16_t contacts_len;
static int16_t *index_by_number = NULL;

static struct db_text_index_t *text_index = NULL;
int16_t db_text_index_len;
static int16_t *text_index_index = NULL;

static int by_number_sort_compar(const void *a, const void *b) {
    return strcmp(contacts[*((int16_t *) a)].number, contacts[*((int16_t *) b)].number);
}

static void build_contact_indexes() {
    if (!contacts_len) return;

    index_by_number = malloc(contacts_len * sizeof(int16_t));
    for (int16_t i=0; i<contacts_len; i++) {
        index_by_number[i] = i;
    }

    qsort(index_by_number, contacts_len, 2, by_number_sort_compar);
}

static ASYNC_DEFINE_FUNCTION(initial_load, void)
    static struct fs_open_data open_data;
    static struct fs_read_data read_data;
    static fs_fd fd;
    static struct fs_close_data close_data;
    static int i;
    static char **fields[2];
    static int j;
    static char **field;
    static uint8_t u8len;
    static int16_t index_position;

    while (1) {

        open_data.path = "contacts";
        ASYNC_AWAIT(fs_open, &open_data);
        if (!fs_fd_is_ok(open_data.ret_fd)) {
            contacts_len = 0;
            break;
        }
        fd = open_data.ret_fd;

        read_data.fd = fd;
        read_data.buff = (uint8_t *) &contacts_len;
        read_data.n = 2;
        ASYNC_AWAIT(fs_read, &read_data);

        if (contacts_len) {
            contacts = malloc(contacts_len * sizeof(struct contact_t));
        }

        i = 0;
        while (1) {
            if (i == contacts_len) break;

            fields[0] = &contacts[i].name;
            fields[1] = &contacts[i].number;
            j = 0;
            while (1) {
                if (j == 2) break;
                field = fields[j];
                read_data.buff = &u8len;
                read_data.n = 1;
                ASYNC_AWAIT(fs_read, &read_data);
                *field = malloc(((size_t) u8len) + 1);
                (*field)[u8len] = '\0';
                read_data.buff = (uint8_t *) *field;
                read_data.n = u8len;
                ASYNC_AWAIT(fs_read, &read_data);
                j++;
            }
            i++;
        }

        close_data.fd = fd;
        ASYNC_AWAIT(fs_close, &close_data);

        break;
    }

    build_contact_indexes();

    while (1) {
        open_data.path = "text_index";
        ASYNC_AWAIT(fs_open, &open_data);
        if (!fs_fd_is_ok(open_data.ret_fd)) {
            db_text_index_len = 0;
            break;
        }
        fd = open_data.ret_fd;

        read_data.fd = fd;
        read_data.buff = (uint8_t *) &db_text_index_len;
        read_data.n = 2;
        ASYNC_AWAIT(fs_read, &read_data);

        if (db_text_index_len) {
            text_index = malloc(db_text_index_len * sizeof(struct db_text_index_t));
            text_index_index = malloc(db_text_index_len * sizeof(int16_t));
        }

        i = 0;
        while (1) {
            if (i == db_text_index_len) break;

            // index position
            read_data.buff = (uint8_t *) &index_position;
            read_data.n = 2;
            ASYNC_AWAIT(fs_read, &read_data);
            text_index_index[i] = index_position;
            // printf("%d\n", (int) index_position);

            // number
            read_data.buff = &u8len;
            read_data.n = 1;
            ASYNC_AWAIT(fs_read, &read_data);
            text_index[i].number = malloc(((size_t) u8len) + 1);
            text_index[i].number[u8len] = '\0';
            read_data.buff = (uint8_t *) text_index[i].number;
            read_data.n = u8len;
            ASYNC_AWAIT(fs_read, &read_data);

            // unreads
            read_data.buff = &text_index[i].unreads;
            read_data.n = 1;
            ASYNC_AWAIT(fs_read, &read_data);

            // timestamp
            read_data.buff = (uint8_t *) text_index[i].timestamp;
            read_data.n = 14;
            ASYNC_AWAIT(fs_read, &read_data);
            // printf("%.14s\n", text_index[i].timestamp);

            // message preview
            read_data.buff = &u8len;
            read_data.n = 1;
            ASYNC_AWAIT(fs_read, &read_data);
            text_index[i].msg_preview = malloc(((size_t) u8len) + 1);
            text_index[i].msg_preview[u8len] = '\0';
            read_data.buff = (uint8_t *) text_index[i].msg_preview;
            read_data.n = u8len;
            ASYNC_AWAIT(fs_read, &read_data);

            i++;
        }

        close_data.fd = fd;
        ASYNC_AWAIT(fs_close, &close_data);

        break;
    }

    // for (int i=0; i<db_text_index_len; i++) {
    //     printf("%.14s\n", text_index[text_index_index[i]].timestamp);
    // }

    async_lock_release(&db_lock);
ASYNC_DEFINE_FUNCTION_END

void db_init() {
    async_spawn(initial_load, NULL);
}

struct bsearch2_ret {
    int index;
    bool found_match;
};
static struct bsearch2_ret bsearch2(const void *key, const void *base, int num, int size, int (*compar)(const void *, const void *)) {
    int a = -1;
    int b = num;
    while (1) {
        if ((b - a) == 1) {
            return (struct bsearch2_ret) { .index=b, .found_match=false };
        }
        int middle = (a + b) / 2;
        int diff = compar(key, base + (middle * size));
        if (diff == 0) {
            // printf("match %d\n", middle);
            return (struct bsearch2_ret) { .index=middle, .found_match=true };
        }
        if (diff < 0) {
            b = middle;
        } else {
            a = middle;
        }
    }
}

static int by_number_search_compar(const void *key, const void *elem) {
    // printf("compare %s to %s (%s)\n", key, contacts[*((int16_t *) elem)].number, contacts[*((int16_t *) elem)].name);
    return strcmp(key, contacts[*((int16_t *) elem)].number);
}

struct db_lookup_res db_contacts_lookup_name_by_number(const char *number) {
    struct bsearch2_ret search_result = bsearch2(number, index_by_number, contacts_len, 2, by_number_search_compar);
    // printf("nows %d\n", search_result.index);
    struct db_lookup_res ret;
    ret.found_match = search_result.found_match;
    if (search_result.found_match) {
        ret.found_match_result = contacts[index_by_number[search_result.index]].name;
        // printf("%s\n", contacts[index_by_number[search_result.index]].name);
    } else {
        ret.no_match_closest_index = search_result.index;
    }
    return ret;
}

struct db_text_index_t *db_get_convo_from_index(int16_t i) {
    return &text_index[text_index_index[i]];
}
