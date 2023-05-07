#include "display_driver.h"
#include "display.h"
#include <stdio.h>
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "pico/stdlib.h"

#define SPI_INST spi_default
#define CS_PIN 20

// static ASYNC_DEFINE_FUNCTION(display_driver_task, void)
//     // ASYNC_ALLOCATE_DATA(void);
    
//     // free(data);
// ASYNC_DEFINE_FUNCTION_END

static uint dma_tx;

static int64_t transmit_alarm_callback(alarm_id_t id, void *user_data) {
    // toggle the VCOM bit
    volatile uint8_t *vcom_byte = display_fb;
    *vcom_byte = *vcom_byte == 0b11000000 ? 0b10000000 : 0b11000000;
    gpio_put(CS_PIN, 1); // CS active high
    dma_channel_set_read_addr(dma_tx, display_fb, true); // true starts it
    return 0; // return zero to not schedule the alarm again
}

static void dma_done_handler() {
    // transmission done
    gpio_put(CS_PIN, 0);
    dma_hw->ints0 = 1u << dma_tx; // clear the interrupt
    // keep the CS pin low for long enough before
    // starting the next transmission
    // 100 ms works, 75 ms doesn't, so use safety factor 2x, 200 ms
    add_alarm_in_us(200, transmit_alarm_callback, NULL, true);
}

void display_driver_init() {
    spi_init(SPI_INST, 2000 * 1000);
    gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
    gpio_init(CS_PIN);
    gpio_set_dir(CS_PIN, GPIO_OUT);
    gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);

    dma_tx = dma_claim_unused_channel(true);

    dma_channel_config c = dma_channel_get_default_config(dma_tx);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    channel_config_set_dreq(&c, spi_get_dreq(spi_default, true));
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, false);
    dma_channel_configure(dma_tx, &c,
                          &spi_get_hw(SPI_INST)->dr, // write address
                          display_fb, // read address
                          FB_LEN, // element count (each element is of size transfer_data_size)
                          false); // don't start yet

    // Tell the DMA to raise IRQ line 0 when the channel finishes a block
    dma_channel_set_irq0_enabled(dma_tx, true);

    // Configure the processor to run dma_done_handler() when DMA IRQ 0 is asserted
    irq_set_exclusive_handler(DMA_IRQ_0, dma_done_handler);
    irq_set_enabled(DMA_IRQ_0, true);

    // Manually call the transmit callback once, to trigger the first transfer
    transmit_alarm_callback(0, NULL);

    // printf("got here\n");

    // while (1) {
    //     display_fb[0] = display_fb[0] == 0b11000000 ? 0b10000000 : 0b11000000;
    //     gpio_put(CS_PIN, 1);
    //     dma_channel_set_read_addr(dma_tx, display_fb, true);
    //     // dma_channel_start(dma_tx);
    //     dma_channel_wait_for_finish_blocking(dma_tx);
    //     gpio_put(CS_PIN, 0);
    //     sleep_ms(5);
    // }

}
