#include "fs.h"
#include "host_bridge.h"
#include <string.h>

void fs_init() {
}

bool fs_fd_is_ok(fs_fd fd) {
    return fd >= 0;
}

ASYNC_DEFINE_FUNCTION(fs_open, struct fs_open_data)
    int path_len = strlen(data->path);
    uint8_t *bytes = malloc(path_len + 1);
    bytes[0] = path_len;
    memcpy(bytes + 1, data->path, path_len);
    host_transmit_data *host_data = malloc(sizeof(host_transmit_data));
    host_data->code = HOST_CODE_FILE_OPEN;
    host_data->bytes = bytes;
    host_data->len = path_len + 1;
    data->extra = host_data;
    ASYNC_AWAIT(host_transmit, host_data);
    host_data = data->extra;
    memcpy(&data->ret_fd, host_data->ret, 4);
    free(host_data->bytes);
    free(host_data->ret);
    free(host_data);
ASYNC_DEFINE_FUNCTION_END


ASYNC_DEFINE_FUNCTION(fs_close, struct fs_close_data)
    host_transmit_data *host_data = malloc(sizeof(host_transmit_data));
    host_data->code = HOST_CODE_FILE_CLOSE;
    host_data->bytes = (uint8_t *) &data->fd;
    host_data->len = 4;
    data->extra = host_data;
    ASYNC_AWAIT(host_transmit, host_data);
    free(data->extra);
ASYNC_DEFINE_FUNCTION_END


ASYNC_DEFINE_FUNCTION(fs_size, struct fs_size_data)
    host_transmit_data *host_data = malloc(sizeof(host_transmit_data));
    host_data->code = HOST_CODE_FILE_SIZE;
    host_data->bytes =  (uint8_t *)  &data->fd;
    host_data->len = 4;
    data->extra = host_data;
    ASYNC_AWAIT(host_transmit, host_data);
    host_data = data->extra;
    memcpy(&data->ret_size, host_data->ret, 4);
    free(host_data->ret);
    free(host_data);
ASYNC_DEFINE_FUNCTION_END


ASYNC_DEFINE_FUNCTION(fs_read, struct fs_read_data)
    host_transmit_data *host_data = malloc(sizeof(host_transmit_data));
    host_data->code = HOST_CODE_FILE_READ;
    uint8_t *bytes = malloc(8);
    memcpy(bytes, &data->fd, 4);
    memcpy(bytes + 4, &data->n, 4);
    host_data->bytes = bytes;
    host_data->len = 8;
    data->extra = host_data;
    ASYNC_AWAIT(host_transmit, host_data);
    host_data = data->extra;
    memcpy(data->buff, host_data->ret, data->n);
    free(host_data->bytes);
    free(host_data->ret);
    free(host_data);
ASYNC_DEFINE_FUNCTION_END

ASYNC_DEFINE_FUNCTION(fs_seek, struct fs_seek_data)
    host_transmit_data *host_data = malloc(sizeof(host_transmit_data));
    host_data->code = HOST_CODE_FILE_SEEK;
    uint8_t *bytes = malloc(9);
    memcpy(bytes, &data->fd, 4);
    memcpy(bytes + 4, &data->n, 4);
    bytes[8] = data->whence;
    host_data->bytes = bytes;
    host_data->len = 9;
    data->extra = host_data;
    ASYNC_AWAIT(host_transmit, host_data);
    host_data = data->extra;
    free(host_data->bytes);
    free(host_data);
ASYNC_DEFINE_FUNCTION_END
