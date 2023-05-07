#include "rotary_encoder_impl.h"
#include "host_bridge.h"
#include <string.h>
#include <stdlib.h>

#if __BYTE_ORDER != __LITTLE_ENDIAN
#error expected little endian architecture
#endif

struct host_and_cb {
    host_transmit_data host_data;
    void (*cb)(int32_t);
};

static ASYNC_DEFINE_FUNCTION(read_host_encoder, struct host_and_cb)
    ASYNC_SLEEP(50);
    ASYNC_AWAIT(host_transmit, &data->host_data);
    int32_t reading;
    memcpy(&reading, data->host_data.ret, 4);
    data->cb(reading);
    free(data->host_data.ret);
    free(data);
ASYNC_DEFINE_FUNCTION_END

void rotary_encoder_read(void (*cb)(int32_t)) {
    struct host_and_cb *encoder_read_data = malloc(sizeof(struct host_and_cb));

    encoder_read_data->cb = cb;

    encoder_read_data->host_data.code = HOST_CODE_ENCODER;
    encoder_read_data->host_data.bytes = NULL;
    encoder_read_data->host_data.len = 0;

    async_spawn(read_host_encoder, encoder_read_data);
}
