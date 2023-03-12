#include "pico/stdlib.h"

void onboard_led_init() {
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

}
void onboard_led_write(bool value) {
    gpio_put(PICO_DEFAULT_LED_PIN, value);
}
