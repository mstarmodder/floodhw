#!/usr/bin/env python3
"""convert a flat binary to a UF2 for RP2350 (Arm Secure)"""
"""flash base 0x10000000"""
import struct, sys

BASE = 0x10000000
FAMILY_RP2350_ARM_S = 0xE48BFF59

data = open(sys.argv[1], "rb").read()
data += b"\x00" * (-len(data) % 256)
n = len(data) // 256

with open(sys.argv[2], "wb") as f:
    for i in range(n):
        chunk = data[i * 256:(i + 1) * 256]
        f.write(struct.pack("<8I", 0x0A324655, 0x9E5D5157, 0x00002000,
                            BASE + i * 256, 256, i, n, FAMILY_RP2350_ARM_S))
        f.write(chunk + b"\x00" * (476 - 256))
        f.write(struct.pack("<I", 0x0AB16F30))
