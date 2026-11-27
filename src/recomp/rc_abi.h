// rc_abi.h - interface between the application and a statically recompiled TMS34010
// program module (tools/recomp). A module is C++ generated from the user's own program
// ROM and compiled into a shared library (recomp/<game>.dll|.so). It is never part of the
// repository or the release packages.
//
// The interface is plain C: the module gets pointers to the CPU state and a function that
// runs any instruction with the interpreter, so it does not depend on the application's
// classes or C++ library and can be built with any C++ compiler (GCC, Clang, Zig).
#pragma once
#include <stdint.h>
#include <stddef.h>

#define RC_ABI_VERSION 4u

typedef struct { int16_t x, y; } rc_xy;                        // little-endian hosts
typedef union { int32_t reg; rc_xy xy; } rc_reg;

struct rc_host
{
	void *cpu;                                   // opaque, passed back to exec
	uint32_t *pc, *ppc, *st;
	int *icount;
	rc_reg *regs;                                // 31 registers: A0-A14, SP, B14-B0
	void (*exec)(void *cpu, uint16_t op);        // interpreter handler for op (PC must point past the opcode)
};

struct rc_module
{
	uint32_t abi;               // RC_ABI_VERSION
	uint64_t rom_hash;          // FNV-1a 64 of the program image the code was generated from
	const char *game;           // catalogue id
	uint32_t base;              // first recompiled bit address (0xff800000)
	uint32_t words;             // 16-bit words covered from base
	uint32_t instructions;      // instructions recompiled
	// Runs recompiled code from *pc until it leaves the recompiled code, the cycle budget
	// runs out or an interrupt redirects it. Returns 0 if *pc is not recompiled code (the
	// caller then interprets one instruction).
	int (*run)(const struct rc_host *h);
};

#ifdef __cplusplus
extern "C" typedef const rc_module *(*rc_get_module_fn)(void);
#endif
#define RC_ENTRY_NAME "mkrc_get_module"

static inline uint64_t rc_fnv64(const uint8_t *p, size_t n)
{
	uint64_t h = 0xcbf29ce484222325ull;
	for (size_t i = 0; i < n; i++) { h ^= p[i]; h *= 0x100000001b3ull; }
	return h;
}
