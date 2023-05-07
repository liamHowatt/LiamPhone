#pragma once

#include "fs_impl_types.h"
#include "async.h"
#include <stdint.h>
#include <stdbool.h>

void fs_init();

bool fs_fd_is_ok(fs_fd fd);

struct fs_open_data {
    const char *path;
    fs_fd ret_fd;
    void *extra;
};
ASYNC_DECLARE_FUNCTION(fs_open, struct fs_open_data);

struct fs_close_data {
    fs_fd fd;
    void *extra;
};
ASYNC_DECLARE_FUNCTION(fs_close, struct fs_close_data);

struct fs_size_data {
    fs_fd fd;
    int ret_size;
    void *extra;
};
ASYNC_DECLARE_FUNCTION(fs_size, struct fs_size_data);

struct fs_read_data {
    fs_fd fd;
    uint8_t *buff;
    int n;
    void *extra;
};
ASYNC_DECLARE_FUNCTION(fs_read, struct fs_read_data);

struct fs_seek_data {
    fs_fd fd;
    int n;
    uint8_t whence;
    void *extra;
};
ASYNC_DECLARE_FUNCTION(fs_seek, struct fs_seek_data);
