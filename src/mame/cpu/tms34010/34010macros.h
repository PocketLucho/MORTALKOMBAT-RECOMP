// license:BSD-3-Clause
// copyright-holders:Alex Pasadyn,Zsolt Vasvari
// 34010macros.h - register / flag / field macros of the TMS34010 core, moved out of
// tms34010.cpp and 34010ops.hxx so that statically recompiled code (tools/recomp) can use
// exactly the same definitions as the interpreter.
#pragma once

/***************************************************************************
    MACROS
***************************************************************************/

/* status register definitions */
#define STBIT_N         (1 << 31)
#define STBIT_C         (1 << 30)
#define STBIT_Z         (1 << 29)
#define STBIT_V         (1 << 28)
#define STBIT_P         (1 << 25)
#define STBIT_IE        (1 << 21)
#define STBIT_FE1       (1 << 11)
#define STBITS_F1       (0x1f << 6)
#define STBIT_FE0       (1 << 5)
#define STBITS_F0       (0x1f << 0)

/* register definitions and shortcuts */
#define N_FLAG()       (m_st & STBIT_N)
#define Z_FLAG()       (m_st & STBIT_Z)
#define C_FLAG()       (m_st & STBIT_C)
#define V_FLAG()       (m_st & STBIT_V)
#define P_FLAG()       (m_st & STBIT_P)
#define IE_FLAG()      (m_st & STBIT_IE)
#define FE0_FLAG()     (m_st & STBIT_FE0)
#define FE1_FLAG()     (m_st & STBIT_FE1)

/* register file access */
#define AREG(i)       (m_regs[i].reg)
#define AREG_XY(i)    (m_regs[i].xy)
#define AREG_X(i)     (m_regs[i].xy.x)
#define AREG_Y(i)     (m_regs[i].xy.y)
#define BREG(i)       (m_regs[30 - (i)].reg)
#define BREG_XY(i)    (m_regs[30 - (i)].xy)
#define BREG_X(i)     (m_regs[30 - (i)].xy.x)
#define BREG_Y(i)     (m_regs[30 - (i)].xy.y)
#define SP()          AREG(15)
#define FW(i)         ((m_st >> (i ? 6 : 0)) & 0x1f)
#define FWEX(i)       ((m_st >> (i ? 6 : 0)) & 0x3f)

/* opcode decode helpers */
#define SRCREG(O)       (((O) >> 5) & 0x0f)
#define DSTREG(O)       ((O) & 0x0f)
#define SKIP_WORD()     (m_pc += (2 << 3))
#define SKIP_LONG()     (m_pc += (4 << 3))
#define PARAM_K(O)      (((O) >> 5) & 0x1f)
#define PARAM_N(O)      ((O) & 0x1f)
#define PARAM_REL8(O)   ((int8_t)(O))

/* memory I/O */
#define WFIELD0(a,b)  (this->*s_wfield_functions[FW(0)])(a,b)
#define WFIELD1(a,b)  (this->*s_wfield_functions[FW(1)])(a,b)
#define RFIELD0(a)    (this->*s_rfield_functions[FWEX(0)])(a)
#define RFIELD1(a)    (this->*s_rfield_functions[FWEX(1)])(a)
#define WPIXEL(a,b)   (this->*m_pixel_write)(a,b)
#define RPIXEL(a)     (this->*m_pixel_read)(a)

/* Implied Operands */
#define SADDR()        BREG(0)
#define SADDR_X()      BREG_X(0)
#define SADDR_Y()      BREG_Y(0)
#define SADDR_XY()     BREG_XY(0)
#define SPTCH()        BREG(1)
#define DADDR()        BREG(2)
#define DADDR_X()      BREG_X(2)
#define DADDR_Y()      BREG_Y(2)
#define DADDR_XY()     BREG_XY(2)
#define DPTCH()        BREG(3)
#define OFFSET()       BREG(4)
#define WSTART_X()     BREG_X(5)
#define WSTART_Y()     BREG_Y(5)
#define WSTART_XY()    BREG_XY(5)
#define WEND_X()       BREG_X(6)
#define WEND_Y()       BREG_Y(6)
#define WEND_XY()      BREG_XY(6)
#define DYDX_X()       BREG_X(7)
#define DYDX_Y()       BREG_Y(7)
#define DYDX_XY()      BREG_XY(7)
#define COLOR0()       BREG(8)
#define COLOR1()       BREG(9)
#define COUNT()        BREG(10)
#define INC1_X()       BREG_X(11)
#define INC1_Y()       BREG_Y(11)
#define INC2_X()       BREG_X(12)
#define INC2_Y()       BREG_Y(12)
#define PATTRN()       BREG(13)
#define TEMP()         BREG(14)

/* I/O registers */
#define WINDOW_CHECKING()  ((IOREG(REG_CONTROL) >> 6) & 0x03)

/***************************************************************************
    MISC MACROS
***************************************************************************/

#define ZEXTEND(val,width) if (width) (val) &= ((uint32_t)0xffffffff >> (32 - (width)))
#define SEXTEND(val,width) if (width) (val) = (int32_t)((val) << (32 - (width))) >> (32 - (width))

#define SXYTOL(val)   ((((int16_t)(val).y * m_convsp) + ((int16_t)(val).x << m_pixelshift)) + OFFSET())
#define DXYTOL(val)   ((((int16_t)(val).y * m_convdp) + ((int16_t)(val).x << m_pixelshift)) + OFFSET())
#define MXYTOL(val)   ((((int16_t)(val).y * m_convmp) + ((int16_t)(val).x << m_pixelshift)) + OFFSET())

#define COUNT_CYCLES(x)   m_icount -= x
#define COUNT_UNKNOWN_CYCLES(x) COUNT_CYCLES(x)

#define CORRECT_ODD_PC(x) do { if (m_pc & 0x0f) logerror("%s to PC=%08X\n", x, m_pc); m_pc &= ~0x0f; } while (0)



/***************************************************************************
    FLAG HANDLING MACROS
***************************************************************************/

#define SIGN(val)           ((val) & 0x80000000)

#define CLR_Z()                m_st &= ~STBIT_Z
#define CLR_V()                m_st &= ~STBIT_V
#define CLR_C()                m_st &= ~STBIT_C
#define CLR_N()                m_st &= ~STBIT_N
#define CLR_NZ()               m_st &= ~(STBIT_N | STBIT_Z)
#define CLR_CZ()               m_st &= ~(STBIT_C | STBIT_Z)
#define CLR_ZV()               m_st &= ~(STBIT_Z | STBIT_V)
#define CLR_NZV()              m_st &= ~(STBIT_N | STBIT_Z | STBIT_V)
#define CLR_NCZ()              m_st &= ~(STBIT_N | STBIT_C | STBIT_Z)
#define CLR_NCZV()             m_st &= ~(STBIT_N | STBIT_C | STBIT_Z | STBIT_V)

#define SET_V_BIT_LO(val,bit) m_st |= ((val) << (28 - (bit))) & STBIT_V
#define SET_V_BIT_HI(val,bit) m_st |= ((val) >> ((bit) - 28)) & STBIT_V
#define SET_V_LOG(val)        m_st |= (val) << 28
#define SET_Z_BIT_LO(val,bit) m_st |= ((val) << (29 - (bit))) & STBIT_Z
#define SET_Z_BIT_HI(val,bit) m_st |= ((val) >> ((bit) - 29)) & STBIT_Z
#define SET_Z_LOG(val)        m_st |= (val) << 29
#define SET_C_BIT_LO(val,bit) m_st |= ((val) << (30 - (bit))) & STBIT_C
#define SET_C_BIT_HI(val,bit) m_st |= ((val) >> ((bit) - 30)) & STBIT_C
#define SET_C_LOG(val)        m_st |= (val) << 30
#define SET_N_BIT(val,bit)    m_st |= ((val) << (31 - (bit))) & STBIT_N
#define SET_N_LOG(val)        m_st |= (val) << 31

#define SET_Z_VAL(val)        SET_Z_LOG((val) == 0)
#define SET_N_VAL(val)        SET_N_BIT(val, 31)
#define SET_NZ_VAL(val)       SET_Z_VAL(val); SET_N_VAL(val)
#define SET_V_SUB(a,b,r)      SET_V_BIT_HI(((a) ^ (b)) & ((a) ^ (r)), 31)
#define SET_V_ADD(a,b,r)      SET_V_BIT_HI(~((a) ^ (b)) & ((a) ^ (r)), 31)
#define SET_C_SUB(a,b)        SET_C_LOG((uint32_t)(b) > (uint32_t)(a))
#define SET_C_ADD(a,b)        SET_C_LOG((uint32_t)~(a) < (uint32_t)(b))
#define SET_NZV_SUB(a,b,r)    SET_NZ_VAL(r); SET_V_SUB(a,b,r)
#define SET_NZCV_SUB(a,b,r)   SET_NZV_SUB(a,b,r); SET_C_SUB(a,b)
#define SET_NZCV_ADD(a,b,r)   SET_NZ_VAL(r); SET_V_ADD(a,b,r); SET_C_ADD(a,b)

static const uint8_t fw_inc[32] = { 32,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31 };

/* instruction macros (from 34010ops.hxx) */
#define ADD_XY(R)                               \
{                                               \
	XY  a =  R##REG_XY(SRCREG(op));                 \
	XY *b = &R##REG_XY(DSTREG(op));                 \
	CLR_NCZV();                                  \
	b->x += a.x;                                \
	b->y += a.y;                                \
	SET_N_LOG(b->x == 0);                      \
	SET_C_BIT_LO(b->y, 15);                        \
	SET_Z_LOG(b->y == 0);                      \
	SET_V_BIT_LO(b->x, 15);                        \
	COUNT_CYCLES(1);                            \
}
#define SUB_XY(R)                               \
{                                               \
	XY  a =  R##REG_XY(SRCREG(op));                 \
	XY *b = &R##REG_XY(DSTREG(op));                 \
	CLR_NCZV();                                  \
	SET_N_LOG(a.x == b->x);                        \
	SET_C_LOG(a.y > b->y);                     \
	SET_Z_LOG(a.y == b->y);                        \
	SET_V_LOG(a.x > b->x);                     \
	b->x -= a.x;                                \
	b->y -= a.y;                                \
	COUNT_CYCLES(1);                            \
}
#define CMP_XY(R)                               \
{                                               \
	int16_t res;                                  \
	XY a = R##REG_XY(DSTREG(op));                   \
	XY b = R##REG_XY(SRCREG(op));                   \
	CLR_NCZV();                                  \
	res = a.x-b.x;                              \
	SET_N_LOG(res == 0);                       \
	SET_V_BIT_LO(res, 15);                     \
	res = a.y-b.y;                              \
	SET_Z_LOG(res == 0);                       \
	SET_C_BIT_LO(res, 15);                     \
	COUNT_CYCLES(1);                            \
}
#define CPW(R)                                  \
{                                               \
	int32_t res = 0;                              \
	int16_t x = R##REG_X(SRCREG(op));                 \
	int16_t y = R##REG_Y(SRCREG(op));                 \
												\
	CLR_V();                                     \
	res |= ((WSTART_X() > x) ? 0x20  : 0);       \
	res |= ((x > WEND_X())   ? 0x40  : 0);       \
	res |= ((WSTART_Y() > y) ? 0x80  : 0);       \
	res |= ((y > WEND_Y())   ? 0x100 : 0);       \
	R##REG(DSTREG(op)) = res;                       \
	SET_V_LOG(res != 0);                       \
	COUNT_CYCLES(1);                            \
}
#define CVXYL(R)                                    \
{                                                   \
	R##REG(DSTREG(op)) = DXYTOL(R##REG_XY(SRCREG(op)));     \
	COUNT_CYCLES(3);                                \
}
#define MOVX(R)                                     \
{                                                   \
	R##REG(DSTREG(op)) = (R##REG(DSTREG(op)) & 0xffff0000) | (uint16_t)R##REG(SRCREG(op));    \
	COUNT_CYCLES(1);                                                                    \
}
#define MOVY(R)                                     \
{                                                   \
	R##REG(DSTREG(op)) = (R##REG(SRCREG(op)) & 0xffff0000) | (uint16_t)R##REG(DSTREG(op));    \
	COUNT_CYCLES(1);                                                                    \
}
#define PIXT_RI(R)                                  \
{                                                   \
	WPIXEL(R##REG(DSTREG(op)),R##REG(SRCREG(op)));  \
	COUNT_UNKNOWN_CYCLES(2);                        \
}
#define PIXT_RIXY(R)                                                                \
{                                                                                   \
	if (WINDOW_CHECKING() != 0)                                                      \
	{                                                                               \
		CLR_V();                                                                     \
		if (R##REG_X(DSTREG(op)) < WSTART_X() || R##REG_X(DSTREG(op)) > WEND_X() ||               \
			R##REG_Y(DSTREG(op)) < WSTART_Y() || R##REG_Y(DSTREG(op)) > WEND_Y())             \
		{                                                                           \
			SET_V_LOG(1);                                                          \
			goto skip;                                                              \
		}                                                                           \
		if (WINDOW_CHECKING() == 1) goto skip;                                       \
	}                                                                               \
	WPIXEL(DXYTOL(R##REG_XY(DSTREG(op))),R##REG(SRCREG(op)));                               \
skip:                                                                               \
	COUNT_UNKNOWN_CYCLES(4);                                                        \
}
#define PIXT_IR(R)                                  \
{                                                   \
	int32_t temp = RPIXEL(R##REG(SRCREG(op)));            \
	CLR_V();                                         \
	R##REG(DSTREG(op)) = temp;                          \
	SET_V_LOG(temp != 0);                          \
	COUNT_CYCLES(4);                                \
}
#define PIXT_II(R)                                  \
{                                                   \
	WPIXEL(R##REG(DSTREG(op)),RPIXEL(R##REG(SRCREG(op))));  \
	COUNT_UNKNOWN_CYCLES(4);                        \
}
#define PIXT_IXYR(R)                                \
{                                                   \
	int32_t temp = RPIXEL(SXYTOL(R##REG_XY(SRCREG(op)))); \
	CLR_V();                                         \
	R##REG(DSTREG(op)) = temp;                          \
	SET_V_LOG(temp != 0);                          \
	COUNT_CYCLES(6);                                \
}
#define PIXT_IXYIXY(R)                                                              \
{                                                                                   \
	if (WINDOW_CHECKING() != 0)                                                      \
	{                                                                               \
		CLR_V();                                                                     \
		if (R##REG_X(DSTREG(op)) < WSTART_X() || R##REG_X(DSTREG(op)) > WEND_X() ||               \
			R##REG_Y(DSTREG(op)) < WSTART_Y() || R##REG_Y(DSTREG(op)) > WEND_Y())             \
		{                                                                           \
			SET_V_LOG(1);                                                          \
			goto skip;                                                              \
		}                                                                           \
		if (WINDOW_CHECKING() == 1) goto skip;                                       \
	}                                                                               \
	WPIXEL(DXYTOL(R##REG_XY(DSTREG(op))),RPIXEL(SXYTOL(R##REG_XY(SRCREG(op)))));            \
skip:                                                                               \
	COUNT_UNKNOWN_CYCLES(7);                                                        \
}
#define DRAV(R)                                                                     \
{                                                                                   \
	if (WINDOW_CHECKING() != 0)                                                      \
	{                                                                               \
		CLR_V();                                                                     \
		if (R##REG_X(DSTREG(op)) < WSTART_X() || R##REG_X(DSTREG(op)) > WEND_X() ||               \
			R##REG_Y(DSTREG(op)) < WSTART_Y() || R##REG_Y(DSTREG(op)) > WEND_Y())             \
		{                                                                           \
			SET_V_LOG(1);                                                          \
			goto skip;                                                              \
		}                                                                           \
		if (WINDOW_CHECKING() == 1) goto skip;                                       \
	}                                                                               \
	WPIXEL(DXYTOL(R##REG_XY(DSTREG(op))),COLOR1());                                      \
skip:                                                                               \
	R##REG_X(DSTREG(op)) += R##REG_X(SRCREG(op));                                           \
	R##REG_Y(DSTREG(op)) += R##REG_Y(SRCREG(op));                                           \
	COUNT_UNKNOWN_CYCLES(4);                                                        \
}
#define ABS(R)                                              \
{                                                           \
	int32_t *rd = &R##REG(DSTREG(op));                            \
	int32_t r = 0 - *rd;                                      \
	CLR_NZV();                                               \
	if (r > 0) *rd = r;                                     \
	SET_NZ_VAL(r);                                         \
	SET_V_LOG(r == (int32_t)0x80000000);                     \
	COUNT_CYCLES(1);                                        \
}
#define ADD(R)                                              \
{                                                           \
	int32_t a = R##REG(SRCREG(op));                               \
	int32_t *rd = &R##REG(DSTREG(op));                            \
	int32_t b = *rd;                                          \
	int32_t r = a + b;                                        \
	CLR_NCZV();                                              \
	*rd = r;                                                \
	SET_NZCV_ADD(a,b,r);                                    \
	COUNT_CYCLES(1);                                        \
}
#define ADDC(R)                                             \
{                                                           \
	/* I'm not sure to which side the carry is added to, should */  \
	/* verify it against the examples */                    \
	int32_t a = R##REG(SRCREG(op));                               \
	int32_t *rd = &R##REG(DSTREG(op));                            \
	int32_t b = *rd;                                          \
	int32_t r = a + b + (C_FLAG() ? 1 : 0);                        \
	CLR_NCZV();                                              \
	*rd = r;                                                \
	SET_NZCV_ADD(a,b,r);                                    \
	COUNT_CYCLES(1);                                        \
}
#define ADDI_W(R)                                           \
{                                                           \
	int32_t a = PARAM_WORD();                              \
	int32_t *rd = &R##REG(DSTREG(op));                            \
	int32_t b = *rd;                                          \
	int32_t r = a + b;                                        \
	CLR_NCZV();                                              \
	*rd = r;                                                \
	SET_NZCV_ADD(a,b,r);                                    \
	COUNT_CYCLES(2);                                        \
}
#define ADDI_L(R)                                           \
{                                                           \
	int32_t a = PARAM_LONG();                              \
	int32_t *rd = &R##REG(DSTREG(op));                            \
	int32_t b = *rd;                                          \
	int32_t r = a + b;                                        \
	CLR_NCZV();                                              \
	*rd = r;                                                \
	SET_NZCV_ADD(a,b,r);                                    \
	COUNT_CYCLES(3);                                        \
}
#define ADDK(R)                                             \
{                                                           \
	int32_t a = fw_inc[PARAM_K(op)];                              \
	int32_t *rd = &R##REG(DSTREG(op));                            \
	int32_t b = *rd;                                          \
	int32_t r = a + b;                                        \
	CLR_NCZV();                                              \
	*rd = r;                                                \
	SET_NZCV_ADD(a,b,r);                                    \
	COUNT_CYCLES(1);                                        \
}
#define AND(R)                                              \
{                                                           \
	int32_t *rd = &R##REG(DSTREG(op));                            \
	CLR_Z();                                                 \
	*rd &= R##REG(SRCREG(op));                                  \
	SET_Z_VAL(*rd);                                            \
	COUNT_CYCLES(1);                                        \
}
#define ANDI(R)                                             \
{                                                           \
	int32_t *rd = &R##REG(DSTREG(op));                            \
	CLR_Z();                                                 \
	*rd &= ~PARAM_LONG();                                \
	SET_Z_VAL(*rd);                                            \
	COUNT_CYCLES(3);                                        \
}
#define ANDN(R)                                             \
{                                                           \
	int32_t *rd = &R##REG(DSTREG(op));                            \
	CLR_Z();                                                 \
	*rd &= ~R##REG(SRCREG(op));                                 \
	SET_Z_VAL(*rd);                                            \
	COUNT_CYCLES(1);                                        \
}
#define BTST_K(R)                                           \
{                                                           \
	int bit = 31 - PARAM_K(op);                                 \
	CLR_Z();                                                 \
	if (bit <= 29)                                          \
		SET_Z_BIT_LO(~R##REG(DSTREG(op)), bit);                    \
	else                                                    \
		SET_Z_BIT_HI(~R##REG(DSTREG(op)), bit);                    \
	COUNT_CYCLES(1);                                        \
}
#define BTST_R(R)                                           \
{                                                           \
	int bit = R##REG(SRCREG(op)) & 0x1f;                        \
	CLR_Z();                                                 \
	if (bit <= 29)                                          \
		SET_Z_BIT_LO(~R##REG(DSTREG(op)), bit);                    \
	else                                                    \
		SET_Z_BIT_HI(~R##REG(DSTREG(op)), bit);                    \
	COUNT_CYCLES(2);                                        \
}
#define CMP(R)                                              \
{                                                           \
	int32_t *rs = &R##REG(SRCREG(op));                            \
	int32_t *rd = &R##REG(DSTREG(op));                            \
	int32_t r = *rd - *rs;                                    \
	CLR_NCZV();                                              \
	SET_NZCV_SUB(*rd,*rs,r);                                \
	COUNT_CYCLES(1);                                        \
}
#define CMPI_W(R)                                           \
{                                                           \
	int32_t *rd = &R##REG(DSTREG(op));                            \
	int32_t t = (int16_t)~PARAM_WORD();                      \
	int32_t r = *rd - t;                                      \
	CLR_NCZV();                                              \
	SET_NZCV_SUB(*rd,t,r);                                  \
	COUNT_CYCLES(2);                                        \
}
#define CMPI_L(R)                                           \
{                                                           \
	int32_t *rd = &R##REG(DSTREG(op));                            \
	int32_t t = ~PARAM_LONG();                             \
	int32_t r = *rd - t;                                      \
	CLR_NCZV();                                              \
	SET_NZCV_SUB(*rd,t,r);                                  \
	COUNT_CYCLES(3);                                        \
}
#define DIVS(R)                                             \
{                                                           \
	int32_t *rs  = &R##REG(SRCREG(op));                           \
	int32_t *rd1 = &R##REG(DSTREG(op));                           \
	CLR_NZV();                                               \
	if (!(DSTREG(op) & 1))                                      \
	{                                                       \
		if (!*rs)                                           \
		{                                                   \
			SET_V_LOG(1);                                  \
		}                                                   \
		else                                                \
		{                                                   \
			int32_t *rd2 = &R##REG(DSTREG(op)+1);                 \
			int64_t dividend = ((uint64_t)*rd1 << 32) | (uint32_t)*rd2; \
			int64_t quotient = dividend / *rs;                \
			int32_t remainder = dividend % *rs;               \
			if ((int64_t)(int32_t)quotient != quotient)     \
			{                                               \
				SET_V_LOG(1);                              \
			}                                               \
			else                                            \
			{                                               \
				*rd1 = quotient;                            \
				*rd2 = remainder;                           \
				SET_NZ_VAL(*rd1);                          \
			}                                               \
		}                                                   \
		COUNT_CYCLES(40);                                   \
	}                                                       \
	else                                                    \
	{                                                       \
		if (!*rs)                                           \
		{                                                   \
			SET_V_LOG(1);                                  \
		}                                                   \
		else                                                \
		{                                                   \
			*rd1 /= *rs;                                    \
			SET_NZ_VAL(*rd1);                              \
		}                                                   \
		COUNT_CYCLES(39);                                   \
	}                                                       \
}
#define DIVU(R)                                             \
{                                                           \
	int32_t *rs  = &R##REG(SRCREG(op));                           \
	int32_t *rd1 = &R##REG(DSTREG(op));                           \
	CLR_ZV();                                                    \
	if (!(DSTREG(op) & 1))                                      \
	{                                                       \
		if (!*rs)                                           \
		{                                                   \
			SET_V_LOG(1);                                  \
		}                                                   \
		else                                                \
		{                                                   \
			int32_t *rd2 = &R##REG(DSTREG(op)+1);                 \
			uint64_t dividend  = ((uint64_t)*rd1 << 32) | (uint32_t)*rd2; \
			uint64_t quotient  = dividend / (uint32_t)*rs;      \
			uint32_t remainder = dividend % (uint32_t)*rs;      \
			if (quotient > 0xffffffff)                      \
			{                                               \
				SET_V_LOG(1);                              \
			}                                               \
			else                                            \
			{                                               \
				*rd1 = quotient;                            \
				*rd2 = remainder;                           \
				SET_Z_VAL(*rd1);                           \
			}                                               \
		}                                                   \
	}                                                       \
	else                                                    \
	{                                                       \
		if (!*rs)                                           \
		{                                                   \
			SET_V_LOG(1);                                  \
		}                                                   \
		else                                                \
		{                                                   \
			*rd1 = (uint32_t)*rd1 / (uint32_t)*rs;              \
			SET_Z_VAL(*rd1);                               \
		}                                                   \
	}                                                       \
	COUNT_CYCLES(37);                                       \
}
#define EXGF(F,R)                                               \
{                                                               \
	uint8_t shift = F ? 6 : 0;                                    \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	uint32_t temp = (m_st >> shift) & 0x3f;                    \
	m_st &= ~(0x3f << shift);                                \
	m_st |= (*rd & 0x3f) << shift;                           \
	*rd = temp;                                                 \
	COUNT_CYCLES(1);                                            \
}
#define LMO(R)                                                  \
{                                                               \
	uint32_t res = 0;                                             \
	uint32_t rs  = R##REG(SRCREG(op));                                \
		int32_t *rd = &R##REG(DSTREG(op));                                \
	CLR_Z();                                                     \
	SET_Z_VAL(rs);                                             \
	if (rs)                                                     \
	{                                                           \
		while (!(rs & 0x80000000))                              \
		{                                                       \
			res++;                                              \
			rs <<= 1;                                           \
		}                                                       \
	}                                                           \
	*rd = res;                                                  \
	COUNT_CYCLES(1);                                            \
}
#define MMFM(R)                                                 \
{                                                               \
	int32_t i;                                                    \
	uint16_t l = (uint16_t) PARAM_WORD();                        \
	COUNT_CYCLES(3);                                            \
	{                                                           \
		int32_t rd = DSTREG(op);                                      \
		for (i = 15; i >= 0 ; i--)                              \
		{                                                       \
			if (l & 0x8000)                                     \
			{                                                   \
				R##REG(i) = RLONG(R##REG(rd));                 \
				R##REG(rd) += 0x20;                             \
				COUNT_CYCLES(4);                                \
			}                                                   \
			l <<= 1;                                            \
		}                                                       \
	}                                                           \
}
#define MMTM(R)                                                 \
{                                                               \
	uint32_t i;                                                   \
	uint16_t l = (uint16_t) PARAM_WORD();                        \
	COUNT_CYCLES(2);                                            \
	{                                                           \
		int32_t rd = DSTREG(op);                                      \
		if (m_is_34020)                                      \
		{                                                       \
			CLR_N();                                             \
			SET_N_VAL(R##REG(rd) ^ 0x80000000);                    \
		}                                                       \
		for (i = 0; i  < 16; i++)                               \
		{                                                       \
			if (l & 0x8000)                                     \
			{                                                   \
				R##REG(rd) -= 0x20;                             \
				WLONG(R##REG(rd),R##REG(i));                   \
				COUNT_CYCLES(4);                                \
			}                                                   \
			l <<= 1;                                            \
		}                                                       \
	}                                                           \
}
#define MODS(R)                                                 \
{                                                               \
	int32_t *rs = &R##REG(SRCREG(op));                                \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	CLR_NZV();                                                   \
	if (*rs != 0)                                               \
	{                                                           \
		*rd %= *rs;                                             \
		SET_NZ_VAL(*rd);                                       \
	}                                                           \
	else                                                        \
		SET_V_LOG(1);                                          \
	COUNT_CYCLES(40);                                           \
}
#define MODU(R)                                                 \
{                                                               \
	int32_t *rs = &R##REG(SRCREG(op));                                \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	CLR_ZV();                                                        \
	if (*rs != 0)                                               \
	{                                                           \
		*rd = (uint32_t)*rd % (uint32_t)*rs;                        \
		SET_Z_VAL(*rd);                                            \
	}                                                           \
	else                                                        \
		SET_V_LOG(1);                                          \
	COUNT_CYCLES(35);                                           \
}
#define MPYS(R)                                                 \
{                                                               \
	int32_t *rd1 = &R##REG(DSTREG(op));                               \
	int32_t m1 = R##REG(SRCREG(op));                                  \
	int64_t product;                                              \
																\
	SEXTEND(m1, FW(1));                                         \
	CLR_NZ();                                                        \
	product = mul_32x32(m1, *rd1);                          \
	SET_Z_LOG(product == 0);                                   \
	SET_N_BIT(product >> 32, 31);                              \
																\
	*rd1             = (int32_t)(product >> 32);                \
	R##REG(DSTREG(op)|1) = product & 0xffffffff;                \
																\
	COUNT_CYCLES(20);                                           \
}
#define MPYU(R)                                                 \
{                                                               \
	int32_t *rd1 = &R##REG(DSTREG(op));                               \
	uint32_t m1 = R##REG(SRCREG(op));                                 \
	uint64_t product;                                             \
																\
	ZEXTEND(m1, FW(1));                                         \
	CLR_Z();                                                     \
	product = mulu_32x32(m1, *rd1);                     \
	SET_Z_LOG(product == 0);                                   \
																\
	*rd1             = (int32_t)(product >> 32);                \
	R##REG(DSTREG(op)|1) = product & 0xffffffff;                \
																\
	COUNT_CYCLES(21);                                           \
}
#define NEG(R)                                                  \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	int32_t r = 0 - *rd;                                          \
	CLR_NCZV();                                                  \
	SET_NZCV_SUB(0,*rd,r);                                      \
	*rd = r;                                                    \
	COUNT_CYCLES(1);                                            \
}
#define NEGB(R)                                                 \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	int32_t t = *rd + (C_FLAG() ? 1 : 0);                          \
	int32_t r = 0 - t;                                            \
	CLR_NCZV();                                                  \
	SET_NZCV_SUB(0,t,r);                                        \
	*rd = r;                                                    \
	COUNT_CYCLES(1);                                            \
}
#define NOT(R)                                                  \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	CLR_Z();                                                     \
	*rd = ~(*rd);                                               \
	SET_Z_VAL(*rd);                                                \
	COUNT_CYCLES(1);                                            \
}
#define OR(R)                                                   \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	CLR_Z();                                                     \
	*rd |= R##REG(SRCREG(op));                                      \
	SET_Z_VAL(*rd);                                                \
	COUNT_CYCLES(1);                                            \
}
#define ORI(R)                                                  \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	CLR_Z();                                                     \
	*rd |= PARAM_LONG();                                     \
	SET_Z_VAL(*rd);                                                \
	COUNT_CYCLES(3);                                            \
}
#define SETF(F)                                                 \
{                                                               \
	uint8_t shift = F ? 6 : 0;                                    \
	m_st &= ~(0x3f << shift);                                \
	m_st |= (op & 0x3f) << shift;                        \
	COUNT_CYCLES(1+F);                                          \
}
#define SEXT(F,R)                                               \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	CLR_NZ();                                                        \
	SEXTEND(*rd,FW(F));                                         \
	SET_NZ_VAL(*rd);                                           \
	COUNT_CYCLES(3);                                            \
}
#define RL(R,K)                                                 \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	int32_t res = *rd;                                            \
	int32_t k = (K);                                              \
	CLR_CZ();                                                        \
	if (k)                                                      \
	{                                                           \
		res<<=(k-1);                                            \
		SET_C_BIT_HI(res, 31);                                 \
		res<<=1;                                                \
		res |= (((uint32_t)*rd)>>((-k)&0x1f));                    \
		*rd = res;                                              \
	}                                                           \
	SET_Z_VAL(res);                                                \
	COUNT_CYCLES(1);                                            \
}
#define SLA(R,K)                                                \
{                                                               \
		int32_t *rd = &R##REG(DSTREG(op));                                \
	uint32_t res = *rd;                                           \
		int32_t k = K;                                                \
	CLR_NCZV();                                                  \
	if (k)                                                      \
	{                                                           \
		uint32_t mask = (0xffffffff<<(31-k))&0x7fffffff;          \
		uint32_t res2 = SIGN(res) ? res^mask : res;               \
		SET_V_LOG((res2 & mask) != 0);                         \
																\
		res<<=(k-1);                                            \
		SET_C_BIT_HI(res, 31);                                 \
		res<<=1;                                                \
		*rd = res;                                              \
	}                                                           \
	SET_NZ_VAL(res);                                           \
	COUNT_CYCLES(3);                                            \
}
#define SLL(R,K)                                                \
{                                                               \
		int32_t *rd = &R##REG(DSTREG(op));                                \
	uint32_t res = *rd;                                           \
		int32_t k = K;                                                \
	CLR_CZ();                                                        \
	if (k)                                                      \
	{                                                           \
		res<<=(k-1);                                            \
		SET_C_BIT_HI(res, 31);                                 \
		res<<=1;                                                \
		*rd = res;                                              \
	}                                                           \
	SET_Z_VAL(res);                                                \
	COUNT_CYCLES(1);                                            \
}
#define SRA(R,K)                                                \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	int32_t res = *rd;                                            \
	int32_t k = (-(K)) & 0x1f;                                    \
	CLR_NCZ();                                                   \
	if (k)                                                      \
	{                                                           \
		res>>=(k-1);                                            \
		SET_C_BIT_LO(res, 0);                                  \
		res>>=1;                                                \
		*rd = res;                                              \
	}                                                           \
	SET_NZ_VAL(res);                                           \
	COUNT_CYCLES(1);                                            \
}
#define SRL(R,K)                                                \
{                                                               \
		int32_t *rd = &R##REG(DSTREG(op));                                \
	uint32_t res = *rd;                                           \
		int32_t k = (-(K)) & 0x1f;                                    \
	CLR_CZ();                                                        \
	if (k)                                                      \
	{                                                           \
		res>>=(k-1);                                            \
		SET_C_BIT_LO(res, 0);                                  \
		res>>=1;                                                \
		*rd = res;                                              \
	}                                                           \
	SET_Z_VAL(res);                                                \
	COUNT_CYCLES(1);                                            \
}
#define SUB(R)                                                  \
{                                                               \
	int32_t *rs = &R##REG(SRCREG(op));                                \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	int32_t r = *rd - *rs;                                        \
	CLR_NCZV();                                                  \
	SET_NZCV_SUB(*rd,*rs,r);                                    \
	*rd = r;                                                    \
	COUNT_CYCLES(1);                                            \
}
#define SUBB(R)                                                 \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	int32_t t = R##REG(SRCREG(op));                                   \
	int32_t r = *rd - t - (C_FLAG() ? 1 : 0);                      \
	CLR_NCZV();                                                  \
	SET_NZCV_SUB(*rd,t,r);                                      \
	*rd = r;                                                    \
	COUNT_CYCLES(1);                                            \
}
#define SUBI_W(R)                                               \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	int32_t r;                                                    \
	int32_t t = ~PARAM_WORD();                                 \
	CLR_NCZV();                                                  \
	r = *rd - t;                                                \
	SET_NZCV_SUB(*rd,t,r);                                      \
	*rd = r;                                                    \
	COUNT_CYCLES(2);                                            \
}
#define SUBI_L(R)                                               \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	int32_t t = ~PARAM_LONG();                                 \
	int32_t r = *rd - t;                                          \
	CLR_NCZV();                                                  \
	SET_NZCV_SUB(*rd,t,r);                                      \
	*rd = r;                                                    \
	COUNT_CYCLES(3);                                            \
}
#define SUBK(R)                                                 \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	int32_t t = fw_inc[PARAM_K(op)];                                  \
	int32_t r = *rd - t;                                          \
	CLR_NCZV();                                                  \
	SET_NZCV_SUB(*rd,t,r);                                      \
	*rd = r;                                                    \
	COUNT_CYCLES(1);                                            \
}
#define XOR(R)                                                  \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	CLR_Z();                                                     \
	*rd ^= R##REG(SRCREG(op));                                      \
	SET_Z_VAL(*rd);                                                \
	COUNT_CYCLES(1);                                            \
}
#define XORI(R)                                                 \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	CLR_Z();                                                     \
	*rd ^= PARAM_LONG();                                     \
	SET_Z_VAL(*rd);                                                \
	COUNT_CYCLES(3);                                            \
}
#define ZEXT(F,R)                                               \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	CLR_Z();                                                     \
	ZEXTEND(*rd,FW(F));                                         \
	SET_Z_VAL(*rd);                                                \
	COUNT_CYCLES(1);                                            \
}
#define MOVI_W(R)                                               \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	CLR_NZV();                                                   \
	*rd=PARAM_WORD();                                        \
	SET_NZ_VAL(*rd);                                           \
	COUNT_CYCLES(2);                                            \
}
#define MOVI_L(R)                                               \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	CLR_NZV();                                                   \
	*rd=PARAM_LONG();                                        \
	SET_NZ_VAL(*rd);                                           \
	COUNT_CYCLES(3);                                            \
}
#define MOVK(R)                                                 \
{                                                               \
	int32_t k = PARAM_K(op); if (!k) k = 32;                          \
	R##REG(DSTREG(op)) = k;                                         \
	COUNT_CYCLES(1);                                            \
}
#define MOVB_RN(R)                                              \
{                                                               \
	WBYTE(R##REG(DSTREG(op)),R##REG(SRCREG(op)));                      \
	COUNT_CYCLES(1);                                            \
}
#define MOVB_NR(R)                                              \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	CLR_NZV();                                                   \
	*rd = (int8_t)RBYTE(R##REG(SRCREG(op)));                     \
	SET_NZ_VAL(*rd);                                           \
	COUNT_CYCLES(3);                                            \
}
#define MOVB_NN(R)                                              \
{                                                               \
	WBYTE(R##REG(DSTREG(op)),(uint32_t)(uint8_t)RBYTE(R##REG(SRCREG(op))));\
	COUNT_CYCLES(3);                                            \
}
#define MOVB_R_NO(R)                                            \
{                                                               \
	int32_t o = PARAM_WORD();                                  \
	WBYTE(R##REG(DSTREG(op))+o,R##REG(SRCREG(op)));                        \
	COUNT_CYCLES(3);                                            \
}
#define MOVB_NO_R(R)                                            \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	int32_t o = PARAM_WORD();                                  \
	CLR_NZV();                                                   \
	*rd = (int8_t)RBYTE(R##REG(SRCREG(op))+o);                   \
	SET_NZ_VAL(*rd);                                           \
	COUNT_CYCLES(5);                                            \
}
#define MOVB_NO_NO(R)                                           \
{                                                               \
	int32_t o1 = PARAM_WORD();                                 \
	int32_t o2 = PARAM_WORD();                                 \
	WBYTE(R##REG(DSTREG(op))+o2,(uint32_t)(uint8_t)RBYTE(R##REG(SRCREG(op))+o1)); \
	COUNT_CYCLES(5);                                            \
}
#define MOVB_RA(R)                                              \
{                                                               \
	WBYTE(PARAM_LONG(),R##REG(DSTREG(op)));                     \
	COUNT_CYCLES(1);                                            \
}
#define MOVB_AR(R)                                              \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	CLR_NZV();                                                   \
	*rd = (int8_t)RBYTE(PARAM_LONG());                    \
	SET_NZ_VAL(*rd);                                           \
	COUNT_CYCLES(5);                                            \
}
#define MOVE_RR(RS,RD)                                          \
{                                                               \
	int32_t *rd = &RD##REG(DSTREG(op));                               \
	CLR_NZV();                                                   \
	*rd = RS##REG(SRCREG(op));                                      \
	SET_NZ_VAL(*rd);                                           \
	COUNT_CYCLES(1);                                            \
}
#define MOVE_RN(F,R)                                            \
{                                                               \
	WFIELD##F(R##REG(DSTREG(op)),R##REG(SRCREG(op)));                   \
	COUNT_CYCLES(1);                                            \
}
#define MOVE_R_DN(F,R)                                          \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	*rd-=fw_inc[FW(F)];                                         \
	WFIELD##F(*rd,R##REG(SRCREG(op)));                              \
	COUNT_CYCLES(2);                                            \
}
#define MOVE_R_NI(F,R)                                          \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	WFIELD##F(*rd,R##REG(SRCREG(op)));                              \
	*rd+=fw_inc[FW(F)];                                         \
	COUNT_CYCLES(1);                                            \
}
#define MOVE_NR(F,R)                                            \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	CLR_NZV();                                                   \
	*rd = RFIELD##F(R##REG(SRCREG(op)));                            \
	SET_NZ_VAL(*rd);                                           \
	COUNT_CYCLES(3);                                            \
}
#define MOVE_DN_R(F,R)                                          \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	int32_t *rs = &R##REG(SRCREG(op));                                \
	CLR_NZV();                                                   \
	*rs-=fw_inc[FW(F)];                                         \
	*rd = RFIELD##F(*rs);                                       \
	SET_NZ_VAL(*rd);                                           \
	COUNT_CYCLES(4);                                            \
}
#define MOVE_NI_R(F,R)                                          \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	int32_t *rs = &R##REG(SRCREG(op));                                \
	int32_t data = RFIELD##F(*rs);                                \
	CLR_NZV();                                                   \
	*rs+=fw_inc[FW(F)];                                         \
	*rd = data;                                                 \
	SET_NZ_VAL(*rd);                                           \
	COUNT_CYCLES(3);                                            \
}
#define MOVE_NN(F,R)                                            \
{                                                               \
	WFIELD##F(R##REG(DSTREG(op)),RFIELD##F(R##REG(SRCREG(op))));        \
	COUNT_CYCLES(3);                                            \
}
#define MOVE_DN_DN(F,R)                                         \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	int32_t *rs = &R##REG(SRCREG(op));                                \
	int32_t data;                                                 \
	*rs-=fw_inc[FW(F)];                                         \
	data = RFIELD##F(*rs);                                      \
	*rd-=fw_inc[FW(F)];                                         \
	WFIELD##F(*rd,data);                                        \
	COUNT_CYCLES(4);                                            \
}
#define MOVE_NI_NI(F,R)                                         \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	int32_t *rs = &R##REG(SRCREG(op));                                \
	int32_t data = RFIELD##F(*rs);                                \
	*rs+=fw_inc[FW(F)];                                         \
	WFIELD##F(*rd,data);                                        \
	*rd+=fw_inc[FW(F)];                                         \
	COUNT_CYCLES(4);                                            \
}
#define MOVE_R_NO(F,R)                                          \
{                                                               \
	int32_t o = PARAM_WORD();                                  \
	WFIELD##F(R##REG(DSTREG(op))+o,R##REG(SRCREG(op)));                 \
	COUNT_CYCLES(3);                                            \
}
#define MOVE_NO_R(F,R)                                          \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	int32_t o = PARAM_WORD();                                  \
	CLR_NZV();                                                   \
	*rd = RFIELD##F(R##REG(SRCREG(op))+o);                          \
	SET_NZ_VAL(*rd);                                           \
	COUNT_CYCLES(5);                                            \
}
#define MOVE_NO_NI(F,R)                                         \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	int32_t o = PARAM_WORD();                                  \
	int32_t data = RFIELD##F(R##REG(SRCREG(op))+o);                   \
	WFIELD##F(*rd,data);                                        \
	*rd+=fw_inc[FW(F)];                                         \
	COUNT_CYCLES(5);                                            \
}
#define MOVE_NO_NO(F,R)                                         \
{                                                               \
	int32_t o1 = PARAM_WORD();                                 \
	int32_t o2 = PARAM_WORD();                                 \
	int32_t data = RFIELD##F(R##REG(SRCREG(op))+o1);                  \
	WFIELD##F(R##REG(DSTREG(op))+o2,data);                          \
	COUNT_CYCLES(5);                                            \
}
#define MOVE_RA(F,R)                                            \
{                                                               \
	WFIELD##F(PARAM_LONG(),R##REG(DSTREG(op)));                  \
	COUNT_CYCLES(3);                                            \
}
#define MOVE_AR(F,R)                                            \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	CLR_NZV();                                                   \
	*rd = RFIELD##F(PARAM_LONG());                           \
	SET_NZ_VAL(*rd);                                           \
	COUNT_CYCLES(5);                                            \
}
#define MOVE_A_NI(F,R)                                          \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	WFIELD##F(*rd,RFIELD##F(PARAM_LONG()));                  \
	*rd+=fw_inc[FW(F)];                                         \
	COUNT_CYCLES(5);                                            \
}
#define MOVE_AA(F)                                              \
{                                                               \
	uint32_t bitaddrs=PARAM_LONG();                            \
	WFIELD##F(PARAM_LONG(),RFIELD##F(bitaddrs));             \
	COUNT_CYCLES(7);                                            \
}
#define CALL(R)                                                 \
{                                                               \
	PUSH(m_pc);                                                 \
	m_pc = R##REG(DSTREG(op));                                       \
	CORRECT_ODD_PC("CALL");                                     \
	COUNT_CYCLES(3);                                            \
}
#define DSJ(R)                                                  \
{                                                               \
	if (--R##REG(DSTREG(op)))                                       \
	{                                                           \
		m_pc += (PARAM_WORD_NO_INC()<<4)+0x10;                    \
		COUNT_CYCLES(3);                                        \
	}                                                           \
	else                                                        \
	{                                                           \
		SKIP_WORD();                                             \
		COUNT_CYCLES(2);                                        \
	}                                                           \
}
#define DSJEQ(R)                                                \
{                                                               \
	if (Z_FLAG())                                                    \
	{                                                           \
		if (--R##REG(DSTREG(op)))                                   \
		{                                                       \
			m_pc += (PARAM_WORD_NO_INC()<<4)+0x10;                \
			COUNT_CYCLES(3);                                    \
		}                                                       \
		else                                                    \
		{                                                       \
			SKIP_WORD();                                         \
			COUNT_CYCLES(2);                                    \
		}                                                       \
	}                                                           \
	else                                                        \
	{                                                           \
		SKIP_WORD();                                             \
		COUNT_CYCLES(2);                                        \
	}                                                           \
}
#define DSJNE(R)                                                \
{                                                               \
	if (!Z_FLAG())                                               \
	{                                                           \
		if (--R##REG(DSTREG(op)))                                   \
		{                                                       \
			m_pc += (PARAM_WORD_NO_INC()<<4)+0x10;                \
			COUNT_CYCLES(3);                                    \
		}                                                       \
		else                                                    \
		{                                                       \
			SKIP_WORD();                                         \
			COUNT_CYCLES(2);                                    \
		}                                                       \
	}                                                           \
	else                                                        \
	{                                                           \
		SKIP_WORD();                                             \
		COUNT_CYCLES(2);                                        \
	}                                                           \
}
#define DSJS(R)                                                 \
{                                                               \
	if (op & 0x0400)                                        \
	{                                                           \
		if (--R##REG(DSTREG(op)))                                   \
		{                                                       \
			m_pc -= ((PARAM_K(op))<<4);                              \
			COUNT_CYCLES(2);                                    \
		}                                                       \
		else                                                    \
			COUNT_CYCLES(3);                                    \
	}                                                           \
	else                                                        \
	{                                                           \
		if (--R##REG(DSTREG(op)))                                   \
		{                                                       \
			m_pc += ((PARAM_K(op))<<4);                              \
			COUNT_CYCLES(2);                                    \
		}                                                       \
		else                                                    \
			COUNT_CYCLES(3);                                    \
	}                                                           \
}
#define EXGPC(R)                                                \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	int32_t temppc = *rd;                                         \
	*rd = m_pc;                                                  \
	m_pc = temppc;                                               \
	CORRECT_ODD_PC("EXGPC");                                    \
	COUNT_CYCLES(2);                                            \
}
#define GETPC(R)                                                \
{                                                               \
	R##REG(DSTREG(op)) = m_pc;                                       \
	COUNT_CYCLES(1);                                            \
}
#define GETST(R)                                                \
{                                                               \
	R##REG(DSTREG(op)) = m_st;                               \
	COUNT_CYCLES(1);                                            \
}
#define j_xx_8(TAKE)                                            \
{                                                               \
	if (DSTREG(op))                                                 \
	{                                                           \
		if (TAKE)                                               \
		{                                                       \
			m_pc += (PARAM_REL8(op) << 4);                           \
			COUNT_CYCLES(2);                                    \
		}                                                       \
		else                                                    \
			COUNT_CYCLES(1);                                    \
	}                                                           \
	else                                                        \
	{                                                           \
		if (TAKE)                                               \
		{                                                       \
			m_pc = PARAM_LONG_NO_INC();                       \
			CORRECT_ODD_PC("J_XX_8");                           \
			COUNT_CYCLES(3);                                    \
		}                                                       \
		else                                                    \
		{                                                       \
			SKIP_LONG();                                         \
			COUNT_CYCLES(4);                                    \
		}                                                       \
	}                                                           \
}
#define j_xx_0(TAKE)                                            \
{                                                               \
	if (DSTREG(op))                                             \
	{                                                           \
		if (TAKE)                                               \
		{                                                       \
			m_pc += (PARAM_REL8(op) << 4);                           \
			COUNT_CYCLES(2);                                    \
		}                                                       \
		else                                                    \
			COUNT_CYCLES(1);                                    \
	}                                                           \
	else                                                        \
	{                                                           \
		if (TAKE)                                               \
		{                                                       \
			m_pc += (PARAM_WORD_NO_INC()<<4)+0x10;                \
			COUNT_CYCLES(3);                                    \
		}                                                       \
		else                                                    \
		{                                                       \
			SKIP_WORD();                                         \
			COUNT_CYCLES(2);                                    \
		}                                                       \
	}                                                           \
}
#define j_xx_x(TAKE)                                            \
{                                                               \
	if (TAKE)                                                   \
	{                                                           \
		m_pc += (PARAM_REL8(op) << 4);                               \
		COUNT_CYCLES(2);                                        \
	}                                                           \
	else                                                        \
		COUNT_CYCLES(1);                                        \
}
#define JUMP(R)                                                 \
{                                                               \
	m_pc = R##REG(DSTREG(op));                                       \
	CORRECT_ODD_PC("JUMP");                                     \
	COUNT_CYCLES(2);                                            \
}
#define PUTST(R)                                                \
{                                                               \
	SET_ST(R##REG(DSTREG(op)));                                \
	COUNT_CYCLES(3);                                            \
}
#define REV(R)                                                  \
{                                                               \
	R##REG(DSTREG(op)) = 0x0008;                                    \
	COUNT_CYCLES(1);                                            \
}
#define ADD_XYI(R)                              \
{                                               \
	uint32_t a = PARAM_LONG();                 \
	XY *b = &R##REG_XY(DSTREG(op));                 \
	CLR_NCZV();                                  \
	b->x += (int16_t)(a & 0xffff);                \
	b->y += ((int32_t)a >> 16);                   \
	SET_N_LOG(b->x == 0);                      \
	SET_C_BIT_LO(b->y, 15);                        \
	SET_Z_LOG(b->y == 0);                      \
	SET_V_BIT_LO(b->x, 15);                        \
	COUNT_CYCLES(1);                            \
}
#define CMPK(R)                                             \
{                                                           \
	int32_t r;                                                \
	int32_t *rd = &R##REG(DSTREG(op));                            \
	int32_t t = PARAM_K(op); if (!t) t = 32;                      \
	CLR_NCZV();                                              \
	r = *rd - t;                                            \
	SET_NZCV_SUB(*rd,t,r);                                  \
	COUNT_CYCLES(1);                                        \
}
#define RMO(R)                                                  \
{                                                               \
	uint32_t res = 0;                                             \
	uint32_t rs  = R##REG(SRCREG(op));                                \
		int32_t *rd = &R##REG(DSTREG(op));                                \
	CLR_Z();                                                     \
	SET_Z_VAL(rs);                                             \
	if (rs)                                                     \
	{                                                           \
		while (!(rs & 0x00000001))                              \
		{                                                       \
			res++;                                              \
			rs >>= 1;                                           \
		}                                                       \
	}                                                           \
	*rd = res;                                                  \
	COUNT_CYCLES(1);                                            \
}
#define RPIX(R)                                 \
{                                               \
	uint32_t v = R##REG(DSTREG(op));                  \
	switch (m_pixelshift)                    \
	{                                           \
		case 0:                                 \
			v = (v & 1) ? 0xffffffff : 0x00000000;\
			COUNT_CYCLES(8);                    \
			break;                              \
		case 1:                                 \
			v &= 3;                             \
			v |= v << 2;                        \
			v |= v << 4;                        \
			v |= v << 8;                        \
			v |= v << 16;                       \
			COUNT_CYCLES(7);                    \
			break;                              \
		case 2:                                 \
			v &= 0x0f;                          \
			v |= v << 4;                        \
			v |= v << 8;                        \
			v |= v << 16;                       \
			COUNT_CYCLES(6);                    \
			break;                              \
		case 3:                                 \
			v &= 0xff;                          \
			v |= v << 8;                        \
			v |= v << 16;                       \
			COUNT_CYCLES(5);                    \
			break;                              \
		case 4:                                 \
			v &= 0xffff;                        \
			v |= v << 16;                       \
			COUNT_CYCLES(4);                    \
			break;                              \
		case 5:                                 \
			COUNT_CYCLES(2);                    \
			break;                              \
	}                                           \
	R##REG(DSTREG(op)) = v;                         \
}
