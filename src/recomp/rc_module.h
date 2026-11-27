// rc_module.h - included by the C++ that mkrecomp generates. Only plain types and the
// TMS34010 core's own macros (34010macros.h), so modules build with any C++ compiler.
#pragma once
#include <stdint.h>
#include "rc_abi.h"
#include "34010macros.h"

#ifdef _WIN32
#define RC_EXPORT extern "C" __declspec(dllexport)
#else
#define RC_EXPORT extern "C" __attribute__((visibility("default")))
#endif

// names the core's macros expect, bound to the host's CPU state
#define RC_BIND(h) \
	uint32_t &m_pc = *(h)->pc; uint32_t &m_ppc = *(h)->ppc; uint32_t &m_st = *(h)->st; \
	int &m_icount = *(h)->icount; rc_reg *const m_regs = (h)->regs; typedef rc_xy XY; \
	void (*const rc_exec)(void *, uint16_t) = (h)->exec; void *const rc_cpu = (h)->cpu; \
	(void)m_ppc; (void)rc_exec; (void)rc_cpu
