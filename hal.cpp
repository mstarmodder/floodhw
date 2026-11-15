// SPDX-License-Identifier: BSD-3-Clause

#include "hal.h"
#include "rp2350.h"

using namespace rp;

namespace hal {

// resets
void unreset(uint32_t mask) {
    reg_clr(RESETS_BASE, mask);
    while ((reg(RESETS_BASE + 0x8) & mask) != mask) {
    }
}

// clocks
void clocks_init() {
    // start the crystal oscillator (~1 ms startup delay, units of 256 cycles)
    reg(XOSC_BASE + 0x0c) = (XOSC_HZ / 1000 + 255) / 256;
    reg(XOSC_BASE + 0x00) = (0xfabu << 12) | 0xaa0u;  // enable, freq_range 1-15 MHz
    while (!(reg(XOSC_BASE + 0x04) & (1u << 31))) {   // stable
    }

    // move clk_sys to clk_ref then clk_ref to XOSC
    reg_clr(CLK_SYS_CTRL, 1);                          // SRC = clk_ref
    while (reg(CLK_SYS_SELECTED) != 1) {
    }
    reg(CLK_REF_DIV)  = 1u << 16;
    reg(CLK_REF_CTRL) = 2;                             // SRC = XOSC
    while (reg(CLK_REF_SELECTED) != (1u << 2)) {
    }

    // pll_sys: 12 MHz / 1 * 125 = 1500 MHz VCO, / 5 / 2 = 150 MHz
    reg_set(RESETS_BASE, RST_PLL_SYS);
    unreset(RST_PLL_SYS);
    reg(PLL_SYS_BASE + 0x00) = 1;                      // REFDIV = 1
    reg(PLL_SYS_BASE + 0x08) = 125;                    // FBDIV_INT
    reg_clr(PLL_SYS_BASE + 0x04, (1u << 0) | (1u << 5));  // power up PLL + VCO
    while (!(reg(PLL_SYS_BASE + 0x00) & (1u << 31))) {    // lock
    }
    reg(PLL_SYS_BASE + 0x0c) = (5u << 16) | (2u << 12);   // POSTDIV1 = 5, POSTDIV2 = 2
    reg_clr(PLL_SYS_BASE + 0x04, 1u << 3);                // power up post dividers

    // clk_sys < PLL_SYS (aux source 0), clk_peri <- clk_sys
    reg(CLK_SYS_DIV)  = 1u << 16;
    reg(CLK_SYS_CTRL) = 1;                             // SRC = aux, AUXSRC = PLL_SYS
    while (reg(CLK_SYS_SELECTED) != 2) {
    }
    reg(CLK_PERI_DIV)  = 1u << 16;
    reg(CLK_PERI_CTRL) = 1u << 11;                     // ENABLE, AUXSRC = clk_sys
}

// gpio
static bool io_ready = false;

static void io_init_once() {
    if (io_ready) return;
    unreset(RST_IO_BANK0 | RST_PADS_BANK0);
    io_ready = true;
}

void gpio_set_function(uint32_t pin, Func f) {
    io_init_once();
    uint32_t pad = PADS_BANK0_BASE + 0x04 + pin * 4;
    reg_clr(pad, (1u << 8) | (1u << 7));               // clear ISO, OD
    reg_set(pad, 1u << 6);                             // IE
    reg(IO_BANK0_BASE + pin * 8 + 4) = f;              // CTRL.FUNCSEL
}

void gpio_init(uint32_t pin) {
    reg(SIO_GPIO_OE_CLR)  = 1u << pin;
    reg(SIO_GPIO_OUT_CLR) = 1u << pin;
    gpio_set_function(pin, FUNC_SIO);
}

void gpio_set_dir(uint32_t pin, bool out) {
    reg(out ? SIO_GPIO_OE_SET : SIO_GPIO_OE_CLR) = 1u << pin;
}

void gpio_set_pull(uint32_t pin, Pull p) {
    uint32_t pad = PADS_BANK0_BASE + 0x04 + pin * 4;
    reg_clr(pad, (1u << 3) | (1u << 2));               // PUE | PDE
    if (p == Pull::Up)   reg_set(pad, 1u << 3);
    if (p == Pull::Down) reg_set(pad, 1u << 2);
}

void gpio_put(uint32_t pin, bool v) {
    reg(v ? SIO_GPIO_OUT_SET : SIO_GPIO_OUT_CLR) = 1u << pin;
}

void gpio_toggle(uint32_t pin) { reg(SIO_GPIO_OUT_XOR) = 1u << pin; }

bool gpio_get(uint32_t pin) { return (reg(SIO_GPIO_IN) >> pin) & 1u; }

// timer
void timer_init() {
    unreset(RST_TIMER0);
    // tick generator
    // clk_ref (12 MHz) / 12 = 1 MHz

    reg(TICKS_BASE + 0x1c) = XOSC_HZ / 1'000'000;      // timer0_cycles
    reg(TICKS_BASE + 0x18) = 1;                        // timer0_ctrl.enable
    while (!(reg(TICKS_BASE + 0x18) & 2)) {            // running
    }
}

uint64_t time_us() {
    uint32_t hi, lo, hi2 = reg(TIMER0_BASE + 0x24);    // timerawh
    do {
        hi  = hi2;
        lo  = reg(TIMER0_BASE + 0x28);                 // timerawl
        hi2 = reg(TIMER0_BASE + 0x24);
    } while (hi != hi2);
    return (uint64_t(hi) << 32) | lo;
}

void sleep_us(uint32_t us) {
    uint32_t start = reg(TIMER0_BASE + 0x28);
    while (uint32_t(reg(TIMER0_BASE + 0x28) - start) < us) {
    }
}

void sleep_ms(uint32_t ms) {
    while (ms--) sleep_us(1000);
}

// uart0
void uart_init(uint32_t baud) {
    unreset(RST_UART0);

    uint32_t div  = (8 * SYS_HZ) / baud;
    uint32_t ibrd = div >> 7;
    uint32_t fbrd = ((div & 0x7f) + 1) / 2;
    if (ibrd == 0) { ibrd = 1; fbrd = 0; }

    reg(UART0_BASE + 0x24) = ibrd;                     // UARTIBRD
    reg(UART0_BASE + 0x28) = fbrd;                     // UARTFBRD
    reg(UART0_BASE + 0x2c) = (3u << 5) | (1u << 4);    // 8N1, FIFO enabled (latches divisors)
    reg(UART0_BASE + 0x30) = (1u << 9) | (1u << 8) | 1u;  // RXE | TXE | UARTEN

    gpio_set_function(0, FUNC_UART);                   // TX
    gpio_set_function(1, FUNC_UART);                   // RX
}

void uart_putc(char c) {
    if (c == '\n') uart_putc('\r');
    while (reg(UART0_BASE + 0x18) & (1u << 5)) {       // TXFF
    }
    reg(UART0_BASE + 0x00) = uint8_t(c);
}

void uart_puts(const char *s) {
    while (*s) uart_putc(*s++);
}

void uart_put_u32(uint32_t v) {
    char buf[10];
    int n = 0;
    do { buf[n++] = char('0' + v % 10); v /= 10; } while (v);
    while (n) uart_putc(buf[--n]);
}

void uart_put_hex(uint32_t v) {
    uart_puts("0x");
    for (int i = 28; i >= 0; i -= 4) uart_putc("0123456789abcdef"[(v >> i) & 0xf]);
}

bool uart_readable() { return !(reg(UART0_BASE + 0x18) & (1u << 4)); }  // !RXFE

char uart_getc() {
    while (!uart_readable()) {
    }
    return char(reg(UART0_BASE + 0x00) & 0xff);
}

}  // namespace hal
