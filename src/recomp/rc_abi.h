// rc_abi.h - interface between the application and a statically recompiled TMS34010
// program module (tools/recomp). A module is C++ generated from the user's own program
// ROM, compiled into a shared library next to the application (recomp/<game>.dll|.so).
// It is never part of the repository or the release packages.
#pragma once
#include <cstdint>

class tms34010_device;

#define RC_ABI_VERSION 3u

struct rc_module
{
	uint32_t abi;               // RC_ABI_VERSION
	uint64_t layout;            // tms34010_device layout hash: module and host built from the same headers
	uint64_t rom_hash;          // FNV-1a 64 of the program image the code was generated from
	const char *game;           // catalogue id
	uint32_t base;              // first recompiled bit address (0xff800000)
	uint32_t words;             // 16-bit words covered from base
	uint32_t instructions;      // instructions recompiled
	// Runs recompiled code from the CPU's current PC until it leaves the recompiled code,
	// the cycle budget runs out or an interrupt redirects it. Returns 0 if the PC is not
	// recompiled code (the caller then interprets one instruction).
	int (*run)(tms34010_device *cpu);
};

extern "C" typedef const rc_module *(*rc_get_module_fn)();
#define RC_ENTRY_NAME "mkrc_get_module"

inline uint64_t rc_fnv64(const uint8_t *p, size_t n)
{
	uint64_t h = 0xcbf29ce484222325ull;
	for (size_t i = 0; i < n; i++) { h ^= p[i]; h *= 0x100000001b3ull; }
	return h;
}
