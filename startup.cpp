// SPDX-License-Identifier: BSD-3-Clause

// vector table, IMAGE_DEF block and reset handler
#include <stdint.h>

extern "C" {
extern uint32_t _sidata, _sdata, _edata, _sbss, _ebss;
extern char _estack;
extern void (*__init_array_start[])();
extern void (*__init_array_end[])();
}

int main();

extern "C" [[noreturn]] void reset_handler() {
    uint32_t *src = &_sidata;                                   // copy .data
    for (uint32_t *dst = &_sdata; dst < &_edata;) *dst++ = *src++;
    for (uint32_t *dst = &_sbss; dst < &_ebss;) *dst++ = 0;     // zero .bss
    for (auto fn = __init_array_start; fn < __init_array_end; fn++) (*fn)();
    main();
    for (;;) {
    }
}

extern "C" [[noreturn]] void default_handler() {
    for (;;) {
    }
}

using handler_t = void (*)();

// must sit at the very start of the image (0x10000000)
__attribute__((section(".vectors"), used))
const handler_t vector_table[16] = {
    reinterpret_cast<handler_t>(&_estack),  // initial SP
    reset_handler,
    default_handler,   // NMI
    default_handler,   // HardFault
    default_handler,   // MemManage
    default_handler,   // BusFault
    default_handler,   // UsageFault
    default_handler,   // SecureFault
    nullptr, nullptr, nullptr,
    default_handler,   // SVCall
    default_handler,   // DebugMon
    nullptr,
    default_handler,   // PendSV
    default_handler,   // SysTick
};

// RP2350 image_def: marks this as a valid Arm Secure executable for the bootrom
__attribute__((section(".embedded_block"), used))
const uint32_t image_def[] = {
    0xffffded3,  // block start
    0x10210142,  // IMAGE_TYPE: EXE | Secure | Arm | RP2350
    0x000001ff,  // LAST item
    0x00000000,  // next block (0 = self)
    0xab123579,  // block end
};
