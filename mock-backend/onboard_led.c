#include <stdio.h>
#include <stdbool.h>

void onboard_led_init() {
    puts("LED INITIALIZED");
}

void onboard_led_write(bool value) {
    puts(value ? "LED ON" : "LED OFF");
}
