// SPDX-License-Identifier: BSD-3-Clause

// tiny hardware abstraction layer
#pragma once
#include <stdint.h>

namespace hal {

constexpr uint32_t XOSC_HZ = 12'000'000;   // Pico 2 crystal
constexpr uint32_t SYS_HZ  = 150'000'000;  // clk_sys/clk_peri after clocks_init()

// clocks/resets
void clocks_init();                 // XOSC > PLL_SYS > 150 MHz clk_sys, clk_peri = clk_sys
void unreset(uint32_t mask);        // release peripherals from reset and wait

// gpio's (pins 0..31)
enum Func : uint8_t {
    FUNC_SPI = 1, FUNC_UART = 2, FUNC_I2C = 3, FUNC_PWM = 4,
    FUNC_SIO = 5, FUNC_PIO0 = 6, FUNC_PIO1 = 7,
};
enum class Pull : uint8_t { None, Up, Down };

void gpio_set_function(uint32_t pin, Func f);
void gpio_init(uint32_t pin);                 // SIO function, input, output low
void gpio_set_dir(uint32_t pin, bool out);
void gpio_set_pull(uint32_t pin, Pull p);
void gpio_put(uint32_t pin, bool v);
void gpio_toggle(uint32_t pin);
bool gpio_get(uint32_t pin);

// timer (1 MHz, 64-bit)
void timer_init();
uint64_t time_us();
void sleep_us(uint32_t us);
void sleep_ms(uint32_t ms);

// uart0(TX = GPIO0, RX = GPIO1)
void uart_init(uint32_t baud);
void uart_putc(char c);
void uart_puts(const char *s);
void uart_put_u32(uint32_t v);
void uart_put_hex(uint32_t v);
bool uart_readable();
char uart_getc();                  // blocking

}  // namespace hal
