// SPDX-License-Identifier: BSD-3-Clause

#include "hal.h"

constexpr uint32_t LED_PIN = 25;  // onboard LED (pico 2)

int main() {
    hal::clocks_init();
    hal::timer_init();
    hal::uart_init(115200);       // TX = GPIO0, RX = GPIO1

    hal::gpio_init(LED_PIN);
    hal::gpio_set_dir(LED_PIN, true);

    hal::uart_puts("\nfloodhw\nclk_sys = ");
    hal::uart_put_u32(hal::SYS_HZ / 1'000'000);
    hal::uart_puts(" MHz\n");

    uint32_t n = 0;
    for (;;) {
        hal::gpio_toggle(LED_PIN);

        // example ticker
        hal::uart_puts("tick ");
        hal::uart_put_u32(n++);
        hal::uart_puts("  uptime ");
        hal::uart_put_u32(uint32_t(hal::time_us() / 1000));
        hal::uart_puts(" ms\n");

        // echo anything that uart recieves
        // disabled because it's optional and i personally dont think it can be used right here in any way that has any point'
        // while (hal::uart_readable()) hal::uart_putc(hal::uart_getc());

        // if you need faster ticks then decrease value here
        // or alternatively increase the value for longer ticks
        // current value is set to one tick being 250ms
        hal::sleep_ms(250);
    }
}
