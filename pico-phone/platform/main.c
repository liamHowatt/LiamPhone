#include "pico/stdlib.h"
#include "core.h"

int main() {
    stdio_init_all();
    printf("power on\n");
    sleep_ms(5000);
    core();
}
