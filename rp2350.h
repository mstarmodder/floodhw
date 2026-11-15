// SPDX-License-Identifier: BSD-3-Clause

// minimal RP2350 register map
#pragma once
#include <stdint.h>

inline volatile uint32_t &reg(uint32_t addr) {
    return *reinterpret_cast<volatile uint32_t *>(addr);
}

// atomic register aliases (work on APB peripherals, not on SIO)
inline void reg_xor(uint32_t addr, uint32_t mask) { reg(addr | 0x1000) = mask; }
inline void reg_set(uint32_t addr, uint32_t mask) { reg(addr | 0x2000) = mask; }
inline void reg_clr(uint32_t addr, uint32_t mask) { reg(addr | 0x3000) = mask; }

namespace rp {

// peripheral base addresses
constexpr uint32_t CLOCKS_BASE     = 0x40010000;
constexpr uint32_t RESETS_BASE     = 0x40020000;
constexpr uint32_t IO_BANK0_BASE   = 0x40028000;
constexpr uint32_t PADS_BANK0_BASE = 0x40038000;
constexpr uint32_t XOSC_BASE       = 0x40048000;
constexpr uint32_t PLL_SYS_BASE    = 0x40050000;
constexpr uint32_t UART0_BASE      = 0x40070000;
constexpr uint32_t TIMER0_BASE     = 0x400b0000;
constexpr uint32_t TICKS_BASE      = 0x40108000;
constexpr uint32_t SIO_BASE        = 0xd0000000;

// resets bits
constexpr uint32_t RST_IO_BANK0   = 1u << 6;
constexpr uint32_t RST_PADS_BANK0 = 1u << 9;
constexpr uint32_t RST_PLL_SYS    = 1u << 14;
constexpr uint32_t RST_TIMER0     = 1u << 23;
constexpr uint32_t RST_UART0      = 1u << 26;

// clocks offsets
constexpr uint32_t CLK_REF_CTRL      = CLOCKS_BASE + 0x30;
constexpr uint32_t CLK_REF_DIV       = CLOCKS_BASE + 0x34;
constexpr uint32_t CLK_REF_SELECTED  = CLOCKS_BASE + 0x38;
constexpr uint32_t CLK_SYS_CTRL      = CLOCKS_BASE + 0x3c;
constexpr uint32_t CLK_SYS_DIV       = CLOCKS_BASE + 0x40;
constexpr uint32_t CLK_SYS_SELECTED  = CLOCKS_BASE + 0x44;
constexpr uint32_t CLK_PERI_CTRL     = CLOCKS_BASE + 0x48;
constexpr uint32_t CLK_PERI_DIV      = CLOCKS_BASE + 0x4c;

// sio offsets (RP2350)
constexpr uint32_t SIO_GPIO_IN      = SIO_BASE + 0x004;
constexpr uint32_t SIO_GPIO_OUT_SET = SIO_BASE + 0x018;
constexpr uint32_t SIO_GPIO_OUT_CLR = SIO_BASE + 0x020;
constexpr uint32_t SIO_GPIO_OUT_XOR = SIO_BASE + 0x028;
constexpr uint32_t SIO_GPIO_OE_SET  = SIO_BASE + 0x038;
constexpr uint32_t SIO_GPIO_OE_CLR  = SIO_BASE + 0x040;

}  // namespace rp
