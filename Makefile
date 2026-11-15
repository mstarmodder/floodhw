PREFIX  = arm-none-eabi-
CXX     = $(PREFIX)g++
OBJCOPY = $(PREFIX)objcopy

CXXFLAGS = -mcpu=cortex-m33 -mthumb -Os -Wall -Wextra \
           -ffreestanding -fno-exceptions -fno-rtti -fno-threadsafe-statics \
           -ffunction-sections -fdata-sections -std=c++17
LDFLAGS  = -nostartfiles -nostdlib -T linker.ld -Wl,--gc-sections
LDLIBS   = -lgcc

all: firmware.uf2

SRCS = startup.cpp hal.cpp main.cpp

firmware.elf: $(SRCS) hal.h rp2350.h linker.ld
	$(CXX) $(CXXFLAGS) $(SRCS) $(LDFLAGS) $(LDLIBS) -o $@

firmware.bin: firmware.elf
	$(OBJCOPY) -O binary $< $@

firmware.uf2: firmware.bin bin2uf2.py
	python3 bin2uf2.py $< $@

clean:
	rm -f firmware.elf firmware.bin firmware.uf2

.PHONY: all clean
