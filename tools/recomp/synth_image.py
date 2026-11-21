#!/usr/bin/env python3
# synth_image.py <out.bin> - a synthetic 1 MB TMS34010 program image for CI: every opcode
# group once (parameter words are NOPs), a pointer to each one so the recompiler
# finds them all, and the reset vector. Used to check that every handler compiles in a
# module on each platform; it is not a runnable program.
import struct, sys
BASE = 0xff800000
img = bytearray(0x100000)
def w(addr, v):               # addr: bit address
    o = ((addr - BASE) >> 4) * 2
    img[o:o + 2] = struct.pack('<H', v & 0xffff)
code = BASE + 0x1000 * 16
ptrs = BASE + 0x40000 * 16
starts = []
a = code
for g in range(4096):
    op = (g << 4) | 1          # low nibble 1: register 1 / short forms
    starts.append(a)
    w(a, op); w(a + 16, 0x0300); w(a + 32, 0x0300); w(a + 48, 0x0300)
    w(a + 64, 0x0300)          # NOP
    w(a + 80, 0x0960)          # RETS
    a += 96
for i, s in enumerate(starts):
    w(ptrs + i * 32, s & 0xffff); w(ptrs + i * 32 + 16, s >> 16)
w(0xffffffe0, code & 0xffff); w(0xfffffff0, code >> 16)
open(sys.argv[1], 'wb').write(img)
