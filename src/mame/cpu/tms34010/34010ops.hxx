// license:BSD-3-Clause
// copyright-holders:Alex Pasadyn,Zsolt Vasvari
/***************************************************************************

    TMS34010: Portable Texas Instruments TMS34010 emulator

    Copyright Alex Pasadyn/Zsolt Vasvari
    Parts based on code by Aaron Giles

***************************************************************************/





/***************************************************************************
    UNIMPLEMENTED INSTRUCTION
***************************************************************************/

void tms340x0_device::unimpl(uint16_t op)
{
	// Not all illegal opcodes cause TRAP 30 exceptions on TMS34010, and some games only work because of this:

	// 9 Ball Shootout calls to FFDF7468, expecting it
	// to execute the next instruction from FFDF7470
	// but the instruction at FFDF7460 is an 0x0001

	// Super High Impact executes 0x0007 at various entry points

	logerror("TMS34010 reserved opcode %04Xh encountered at %08x\n", op, m_ppc);
	COUNT_CYCLES(1);
}

void tms340x0_device::illop(uint16_t op)
{
	PUSH(m_pc);
	PUSH(m_st);
	RESET_ST();
	m_pc = RLONG(0xfffffc20);
	COUNT_UNKNOWN_CYCLES(16);

	/* extra check to prevent bad things */
#if 0
	if (m_pc == 0 || s_opcode_table[space(AS_PROGRAM).read_word(m_pc) >> 4] == &tms34010_device::unimpl)
	{
		set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
		machine().debug_break();
	}
#endif
}


/***************************************************************************
    X/Y OPERATIONS
***************************************************************************/

void tms340x0_device::add_xy_a(uint16_t op) { ADD_XY(A); }
void tms340x0_device::add_xy_b(uint16_t op) { ADD_XY(B); }

void tms340x0_device::sub_xy_a(uint16_t op) { SUB_XY(A); }
void tms340x0_device::sub_xy_b(uint16_t op) { SUB_XY(B); }

void tms340x0_device::cmp_xy_a(uint16_t op) { CMP_XY(A); }
void tms340x0_device::cmp_xy_b(uint16_t op) { CMP_XY(B); }

void tms340x0_device::cpw_a(uint16_t op) { CPW(A); }
void tms340x0_device::cpw_b(uint16_t op) { CPW(B); }

// TODO: Handle cycles properly, currently we assume power of two
void tms340x0_device::cvxyl_a(uint16_t op) { CVXYL(A); }
void tms340x0_device::cvxyl_b(uint16_t op) { CVXYL(B); }

void tms340x0_device::movx_a(uint16_t op) { MOVX(A); }
void tms340x0_device::movx_b(uint16_t op) { MOVX(B); }

void tms340x0_device::movy_a(uint16_t op) { MOVY(A); }
void tms340x0_device::movy_b(uint16_t op) { MOVY(B); }



/***************************************************************************
    PIXEL TRANSFER OPERATIONS
***************************************************************************/

void tms340x0_device::pixt_ri_a(uint16_t op) { PIXT_RI(A); }
void tms340x0_device::pixt_ri_b(uint16_t op) { PIXT_RI(B); }

void tms340x0_device::pixt_rixy_a(uint16_t op) { PIXT_RIXY(A); }
void tms340x0_device::pixt_rixy_b(uint16_t op) { PIXT_RIXY(B); }

void tms340x0_device::pixt_ir_a(uint16_t op) { PIXT_IR(A); }
void tms340x0_device::pixt_ir_b(uint16_t op) { PIXT_IR(B); }

void tms340x0_device::pixt_ii_a(uint16_t op) { PIXT_II(A); }
void tms340x0_device::pixt_ii_b(uint16_t op) { PIXT_II(B); }

void tms340x0_device::pixt_ixyr_a(uint16_t op) { PIXT_IXYR(A); }
void tms340x0_device::pixt_ixyr_b(uint16_t op) { PIXT_IXYR(B); }

void tms340x0_device::pixt_ixyixy_a(uint16_t op) { PIXT_IXYIXY(A); }
void tms340x0_device::pixt_ixyixy_b(uint16_t op) { PIXT_IXYIXY(B); }

void tms340x0_device::drav_a(uint16_t op) { DRAV(A); }
void tms340x0_device::drav_b(uint16_t op) { DRAV(B); }



/***************************************************************************
    ARITHMETIC OPERATIONS
***************************************************************************/

void tms340x0_device::abs_a(uint16_t op) { ABS(A); }
void tms340x0_device::abs_b(uint16_t op) { ABS(B); }

void tms340x0_device::add_a(uint16_t op) { ADD(A); }
void tms340x0_device::add_b(uint16_t op) { ADD(B); }

void tms340x0_device::addc_a(uint16_t op) { ADDC(A); }
void tms340x0_device::addc_b(uint16_t op) { ADDC(B); }

void tms340x0_device::addi_w_a(uint16_t op) { ADDI_W(A); }
void tms340x0_device::addi_w_b(uint16_t op) { ADDI_W(B); }

void tms340x0_device::addi_l_a(uint16_t op) { ADDI_L(A); }
void tms340x0_device::addi_l_b(uint16_t op) { ADDI_L(B); }

void tms340x0_device::addk_a(uint16_t op) { ADDK(A); }
void tms340x0_device::addk_b(uint16_t op) { ADDK(B); }

void tms340x0_device::and_a(uint16_t op) { AND(A); }
void tms340x0_device::and_b(uint16_t op) { AND(B); }

void tms340x0_device::andi_a(uint16_t op) { ANDI(A); }
void tms340x0_device::andi_b(uint16_t op) { ANDI(B); }

void tms340x0_device::andn_a(uint16_t op) { ANDN(A); }
void tms340x0_device::andn_b(uint16_t op) { ANDN(B); }

void tms340x0_device::btst_k_a(uint16_t op) { BTST_K(A); }
void tms340x0_device::btst_k_b(uint16_t op) { BTST_K(B); }

void tms340x0_device::btst_r_a(uint16_t op) { BTST_R(A); }
void tms340x0_device::btst_r_b(uint16_t op) { BTST_R(B); }

void tms340x0_device::clrc(uint16_t op)
{
	CLR_C();
	COUNT_CYCLES(1);
}

void tms340x0_device::cmp_a(uint16_t op) { CMP(A); }
void tms340x0_device::cmp_b(uint16_t op) { CMP(B); }

void tms340x0_device::cmpi_w_a(uint16_t op) { CMPI_W(A); }
void tms340x0_device::cmpi_w_b(uint16_t op) { CMPI_W(B); }

void tms340x0_device::cmpi_l_a(uint16_t op) { CMPI_L(A); }
void tms340x0_device::cmpi_l_b(uint16_t op) { CMPI_L(B); }

void tms340x0_device::dint(uint16_t op)
{
	m_st &= ~STBIT_IE;
	COUNT_CYCLES(3);
}

void tms340x0_device::divs_a(uint16_t op) { DIVS(A); }
void tms340x0_device::divs_b(uint16_t op) { DIVS(B); }

void tms340x0_device::divu_a(uint16_t op) { DIVU(A); }
void tms340x0_device::divu_b(uint16_t op) { DIVU(B); }

void tms340x0_device::eint(uint16_t op)
{
	m_st |= STBIT_IE;
	check_interrupt();
	COUNT_CYCLES(3);
}

void tms340x0_device::exgf0_a(uint16_t op) { EXGF(0,A); }
void tms340x0_device::exgf0_b(uint16_t op) { EXGF(0,B); }
void tms340x0_device::exgf1_a(uint16_t op) { EXGF(1,A); }
void tms340x0_device::exgf1_b(uint16_t op) { EXGF(1,B); }

void tms340x0_device::lmo_a(uint16_t op) { LMO(A); }
void tms340x0_device::lmo_b(uint16_t op) { LMO(B); }

void tms340x0_device::mmfm_a(uint16_t op) { MMFM(A); }
void tms340x0_device::mmfm_b(uint16_t op) { MMFM(B); }

void tms340x0_device::mmtm_a(uint16_t op) { MMTM(A); }
void tms340x0_device::mmtm_b(uint16_t op) { MMTM(B); }

void tms340x0_device::mods_a(uint16_t op) { MODS(A); }
void tms340x0_device::mods_b(uint16_t op) { MODS(B); }

void tms340x0_device::modu_a(uint16_t op) { MODU(A); }
void tms340x0_device::modu_b(uint16_t op) { MODU(B); }

void tms340x0_device::mpys_a(uint16_t op) { MPYS(A); }
void tms340x0_device::mpys_b(uint16_t op) { MPYS(B); }

void tms340x0_device::mpyu_a(uint16_t op) { MPYU(A); }
void tms340x0_device::mpyu_b(uint16_t op) { MPYU(B); }

void tms340x0_device::neg_a(uint16_t op) { NEG(A); }
void tms340x0_device::neg_b(uint16_t op) { NEG(B); }

void tms340x0_device::negb_a(uint16_t op) { NEGB(A); }
void tms340x0_device::negb_b(uint16_t op) { NEGB(B); }

void tms340x0_device::nop(uint16_t op)
{
	COUNT_CYCLES(1);
}

void tms340x0_device::not_a(uint16_t op) { NOT(A); }
void tms340x0_device::not_b(uint16_t op) { NOT(B); }

void tms340x0_device::or_a(uint16_t op) { OR(A); }
void tms340x0_device::or_b(uint16_t op) { OR(B); }

void tms340x0_device::ori_a(uint16_t op) { ORI(A); }
void tms340x0_device::ori_b(uint16_t op) { ORI(B); }

void tms340x0_device::setc(uint16_t op)
{
	SET_C_LOG(1);
	COUNT_CYCLES(1);
}

void tms340x0_device::setf0(uint16_t op) { SETF(0); }
void tms340x0_device::setf1(uint16_t op) { SETF(1); }

void tms340x0_device::sext0_a(uint16_t op) { SEXT(0,A); }
void tms340x0_device::sext0_b(uint16_t op) { SEXT(0,B); }
void tms340x0_device::sext1_a(uint16_t op) { SEXT(1,A); }
void tms340x0_device::sext1_b(uint16_t op) { SEXT(1,B); }

void tms340x0_device::rl_k_a(uint16_t op) { RL(A,PARAM_K(op)); }
void tms340x0_device::rl_k_b(uint16_t op) { RL(B,PARAM_K(op)); }
void tms340x0_device::rl_r_a(uint16_t op) { RL(A,AREG(SRCREG(op))&0x1f); }
void tms340x0_device::rl_r_b(uint16_t op) { RL(B,BREG(SRCREG(op))&0x1f); }

void tms340x0_device::sla_k_a(uint16_t op) { SLA(A,PARAM_K(op)); }
void tms340x0_device::sla_k_b(uint16_t op) { SLA(B,PARAM_K(op)); }
void tms340x0_device::sla_r_a(uint16_t op) { SLA(A,AREG(SRCREG(op))&0x1f); }
void tms340x0_device::sla_r_b(uint16_t op) { SLA(B,BREG(SRCREG(op))&0x1f); }

void tms340x0_device::sll_k_a(uint16_t op) { SLL(A,PARAM_K(op)); }
void tms340x0_device::sll_k_b(uint16_t op) { SLL(B,PARAM_K(op)); }
void tms340x0_device::sll_r_a(uint16_t op) { SLL(A,AREG(SRCREG(op))&0x1f); }
void tms340x0_device::sll_r_b(uint16_t op) { SLL(B,BREG(SRCREG(op))&0x1f); }

void tms340x0_device::sra_k_a(uint16_t op) { SRA(A,PARAM_K(op)); }
void tms340x0_device::sra_k_b(uint16_t op) { SRA(B,PARAM_K(op)); }
void tms340x0_device::sra_r_a(uint16_t op) { SRA(A,AREG(SRCREG(op))); }
void tms340x0_device::sra_r_b(uint16_t op) { SRA(B,BREG(SRCREG(op))); }

void tms340x0_device::srl_k_a(uint16_t op) { SRL(A,PARAM_K(op)); }
void tms340x0_device::srl_k_b(uint16_t op) { SRL(B,PARAM_K(op)); }
void tms340x0_device::srl_r_a(uint16_t op) { SRL(A,AREG(SRCREG(op))); }
void tms340x0_device::srl_r_b(uint16_t op) { SRL(B,BREG(SRCREG(op))); }

void tms340x0_device::sub_a(uint16_t op) { SUB(A); }
void tms340x0_device::sub_b(uint16_t op) { SUB(B); }

void tms340x0_device::subb_a(uint16_t op) { SUBB(A); }
void tms340x0_device::subb_b(uint16_t op) { SUBB(B); }

void tms340x0_device::subi_w_a(uint16_t op) { SUBI_W(A); }
void tms340x0_device::subi_w_b(uint16_t op) { SUBI_W(B); }

void tms340x0_device::subi_l_a(uint16_t op) { SUBI_L(A); }
void tms340x0_device::subi_l_b(uint16_t op) { SUBI_L(B); }

void tms340x0_device::subk_a(uint16_t op) { SUBK(A); }
void tms340x0_device::subk_b(uint16_t op) { SUBK(B); }

void tms340x0_device::xor_a(uint16_t op) { XOR(A); }
void tms340x0_device::xor_b(uint16_t op) { XOR(B); }

void tms340x0_device::xori_a(uint16_t op) { XORI(A); }
void tms340x0_device::xori_b(uint16_t op) { XORI(B); }

void tms340x0_device::zext0_a(uint16_t op) { ZEXT(0,A); }
void tms340x0_device::zext0_b(uint16_t op) { ZEXT(0,B); }
void tms340x0_device::zext1_a(uint16_t op) { ZEXT(1,A); }
void tms340x0_device::zext1_b(uint16_t op) { ZEXT(1,B); }



/***************************************************************************
    MOVE INSTRUCTIONS
***************************************************************************/

void tms340x0_device::movi_w_a(uint16_t op) { MOVI_W(A); }
void tms340x0_device::movi_w_b(uint16_t op) { MOVI_W(B); }

void tms340x0_device::movi_l_a(uint16_t op) { MOVI_L(A); }
void tms340x0_device::movi_l_b(uint16_t op) { MOVI_L(B); }

void tms340x0_device::movk_a(uint16_t op) { MOVK(A); }
void tms340x0_device::movk_b(uint16_t op) { MOVK(B); }

void tms340x0_device::movb_rn_a(uint16_t op) { MOVB_RN(A); }
void tms340x0_device::movb_rn_b(uint16_t op) { MOVB_RN(B); }

void tms340x0_device::movb_nr_a(uint16_t op) { MOVB_NR(A); }
void tms340x0_device::movb_nr_b(uint16_t op) { MOVB_NR(B); }

void tms340x0_device::movb_nn_a(uint16_t op) { MOVB_NN(A); }
void tms340x0_device::movb_nn_b(uint16_t op) { MOVB_NN(B); }

void tms340x0_device::movb_r_no_a(uint16_t op) { MOVB_R_NO(A); }
void tms340x0_device::movb_r_no_b(uint16_t op) { MOVB_R_NO(B); }

void tms340x0_device::movb_no_r_a(uint16_t op) { MOVB_NO_R(A); }
void tms340x0_device::movb_no_r_b(uint16_t op) { MOVB_NO_R(B); }

void tms340x0_device::movb_no_no_a(uint16_t op) { MOVB_NO_NO(A); }
void tms340x0_device::movb_no_no_b(uint16_t op) { MOVB_NO_NO(B); }

void tms340x0_device::movb_ra_a(uint16_t op) { MOVB_RA(A); }
void tms340x0_device::movb_ra_b(uint16_t op) { MOVB_RA(B); }

void tms340x0_device::movb_ar_a(uint16_t op) { MOVB_AR(A); }
void tms340x0_device::movb_ar_b(uint16_t op) { MOVB_AR(B); }

void tms340x0_device::movb_aa(uint16_t op)
{
	uint32_t bitaddrs=PARAM_LONG();
	WBYTE(PARAM_LONG(),(uint32_t)(uint8_t)RBYTE(bitaddrs));
	COUNT_CYCLES(6);
}

void tms340x0_device::move_rr_a (uint16_t op) { MOVE_RR(A,A); }
void tms340x0_device::move_rr_b (uint16_t op) { MOVE_RR(B,B); }
void tms340x0_device::move_rr_ax(uint16_t op) { MOVE_RR(A,B); }
void tms340x0_device::move_rr_bx(uint16_t op) { MOVE_RR(B,A); }

void tms340x0_device::move0_rn_a (uint16_t op) { MOVE_RN(0,A); }
void tms340x0_device::move0_rn_b (uint16_t op) { MOVE_RN(0,B); }
void tms340x0_device::move1_rn_a (uint16_t op) { MOVE_RN(1,A); }
void tms340x0_device::move1_rn_b (uint16_t op) { MOVE_RN(1,B); }

void tms340x0_device::move0_r_dn_a (uint16_t op) { MOVE_R_DN(0,A); }
void tms340x0_device::move0_r_dn_b (uint16_t op) { MOVE_R_DN(0,B); }
void tms340x0_device::move1_r_dn_a (uint16_t op) { MOVE_R_DN(1,A); }
void tms340x0_device::move1_r_dn_b (uint16_t op) { MOVE_R_DN(1,B); }

void tms340x0_device::move0_r_ni_a (uint16_t op) { MOVE_R_NI(0,A); }
void tms340x0_device::move0_r_ni_b (uint16_t op) { MOVE_R_NI(0,B); }
void tms340x0_device::move1_r_ni_a (uint16_t op) { MOVE_R_NI(1,A); }
void tms340x0_device::move1_r_ni_b (uint16_t op) { MOVE_R_NI(1,B); }

void tms340x0_device::move0_nr_a (uint16_t op) { MOVE_NR(0,A); }
void tms340x0_device::move0_nr_b (uint16_t op) { MOVE_NR(0,B); }
void tms340x0_device::move1_nr_a (uint16_t op) { MOVE_NR(1,A); }
void tms340x0_device::move1_nr_b (uint16_t op) { MOVE_NR(1,B); }

void tms340x0_device::move0_dn_r_a (uint16_t op) { MOVE_DN_R(0,A); }
void tms340x0_device::move0_dn_r_b (uint16_t op) { MOVE_DN_R(0,B); }
void tms340x0_device::move1_dn_r_a (uint16_t op) { MOVE_DN_R(1,A); }
void tms340x0_device::move1_dn_r_b (uint16_t op) { MOVE_DN_R(1,B); }

void tms340x0_device::move0_ni_r_a (uint16_t op) { MOVE_NI_R(0,A); }
void tms340x0_device::move0_ni_r_b (uint16_t op) { MOVE_NI_R(0,B); }
void tms340x0_device::move1_ni_r_a (uint16_t op) { MOVE_NI_R(1,A); }
void tms340x0_device::move1_ni_r_b (uint16_t op) { MOVE_NI_R(1,B); }

void tms340x0_device::move0_nn_a (uint16_t op) { MOVE_NN(0,A); }
void tms340x0_device::move0_nn_b (uint16_t op) { MOVE_NN(0,B); }
void tms340x0_device::move1_nn_a (uint16_t op) { MOVE_NN(1,A); }
void tms340x0_device::move1_nn_b (uint16_t op) { MOVE_NN(1,B); }

void tms340x0_device::move0_dn_dn_a (uint16_t op) { MOVE_DN_DN(0,A); }
void tms340x0_device::move0_dn_dn_b (uint16_t op) { MOVE_DN_DN(0,B); }
void tms340x0_device::move1_dn_dn_a (uint16_t op) { MOVE_DN_DN(1,A); }
void tms340x0_device::move1_dn_dn_b (uint16_t op) { MOVE_DN_DN(1,B); }

void tms340x0_device::move0_ni_ni_a (uint16_t op) { MOVE_NI_NI(0,A); }
void tms340x0_device::move0_ni_ni_b (uint16_t op) { MOVE_NI_NI(0,B); }
void tms340x0_device::move1_ni_ni_a (uint16_t op) { MOVE_NI_NI(1,A); }
void tms340x0_device::move1_ni_ni_b (uint16_t op) { MOVE_NI_NI(1,B); }

void tms340x0_device::move0_r_no_a (uint16_t op) { MOVE_R_NO(0,A); }
void tms340x0_device::move0_r_no_b (uint16_t op) { MOVE_R_NO(0,B); }
void tms340x0_device::move1_r_no_a (uint16_t op) { MOVE_R_NO(1,A); }
void tms340x0_device::move1_r_no_b (uint16_t op) { MOVE_R_NO(1,B); }

void tms340x0_device::move0_no_r_a (uint16_t op) { MOVE_NO_R(0,A); }
void tms340x0_device::move0_no_r_b (uint16_t op) { MOVE_NO_R(0,B); }
void tms340x0_device::move1_no_r_a (uint16_t op) { MOVE_NO_R(1,A); }
void tms340x0_device::move1_no_r_b (uint16_t op) { MOVE_NO_R(1,B); }

void tms340x0_device::move0_no_ni_a (uint16_t op) { MOVE_NO_NI(0,A); }
void tms340x0_device::move0_no_ni_b (uint16_t op) { MOVE_NO_NI(0,B); }
void tms340x0_device::move1_no_ni_a (uint16_t op) { MOVE_NO_NI(1,A); }
void tms340x0_device::move1_no_ni_b (uint16_t op) { MOVE_NO_NI(1,B); }

void tms340x0_device::move0_no_no_a (uint16_t op) { MOVE_NO_NO(0,A); }
void tms340x0_device::move0_no_no_b (uint16_t op) { MOVE_NO_NO(0,B); }
void tms340x0_device::move1_no_no_a (uint16_t op) { MOVE_NO_NO(1,A); }
void tms340x0_device::move1_no_no_b (uint16_t op) { MOVE_NO_NO(1,B); }

void tms340x0_device::move0_ra_a (uint16_t op) { MOVE_RA(0,A); }
void tms340x0_device::move0_ra_b (uint16_t op) { MOVE_RA(0,B); }
void tms340x0_device::move1_ra_a (uint16_t op) { MOVE_RA(1,A); }
void tms340x0_device::move1_ra_b (uint16_t op) { MOVE_RA(1,B); }

void tms340x0_device::move0_ar_a (uint16_t op) { MOVE_AR(0,A); }
void tms340x0_device::move0_ar_b (uint16_t op) { MOVE_AR(0,B); }
void tms340x0_device::move1_ar_a (uint16_t op) { MOVE_AR(1,A); }
void tms340x0_device::move1_ar_b (uint16_t op) { MOVE_AR(1,B); }

void tms340x0_device::move0_a_ni_a (uint16_t op) { MOVE_A_NI(0,A); }
void tms340x0_device::move0_a_ni_b (uint16_t op) { MOVE_A_NI(0,B); }
void tms340x0_device::move1_a_ni_a (uint16_t op) { MOVE_A_NI(1,A); }
void tms340x0_device::move1_a_ni_b (uint16_t op) { MOVE_A_NI(1,B); }

void tms340x0_device::move0_aa (uint16_t op) { MOVE_AA(0); }
void tms340x0_device::move1_aa (uint16_t op) { MOVE_AA(1); }



/***************************************************************************
    PROGRAM CONTROL INSTRUCTIONS
***************************************************************************/

void tms340x0_device::call_a (uint16_t op) { CALL(A); }
void tms340x0_device::call_b (uint16_t op) { CALL(B); }

void tms340x0_device::callr(uint16_t op)
{
	PUSH(m_pc+0x10);
	m_pc += (PARAM_WORD_NO_INC()<<4)+0x10;
	COUNT_CYCLES(3);
}

void tms340x0_device::calla(uint16_t op)
{
	PUSH(m_pc+0x20);
	m_pc = PARAM_LONG_NO_INC();
	CORRECT_ODD_PC("CALLA");
	COUNT_CYCLES(4);
}

void tms340x0_device::dsj_a (uint16_t op) { DSJ(A); }
void tms340x0_device::dsj_b (uint16_t op) { DSJ(B); }

void tms340x0_device::dsjeq_a (uint16_t op) { DSJEQ(A); }
void tms340x0_device::dsjeq_b (uint16_t op) { DSJEQ(B); }

void tms340x0_device::dsjne_a (uint16_t op) { DSJNE(A); }
void tms340x0_device::dsjne_b (uint16_t op) { DSJNE(B); }

void tms340x0_device::dsjs_a (uint16_t op) { DSJS(A); }
void tms340x0_device::dsjs_b (uint16_t op) { DSJS(B); }

void tms340x0_device::emu(uint16_t op)
{
	/* in RUN state, this instruction is a NOP */
	COUNT_CYCLES(6);
}

void tms340x0_device::exgpc_a (uint16_t op) { EXGPC(A); }
void tms340x0_device::exgpc_b (uint16_t op) { EXGPC(B); }

void tms340x0_device::getpc_a (uint16_t op) { GETPC(A); }
void tms340x0_device::getpc_b (uint16_t op) { GETPC(B); }

void tms340x0_device::getst_a (uint16_t op) { GETST(A); }
void tms340x0_device::getst_b (uint16_t op) { GETST(B); }




void tms340x0_device::j_UC_0(uint16_t op)
{
	j_xx_0(1);
}
void tms340x0_device::j_UC_8(uint16_t op)
{
	j_xx_8(1);
}
void tms340x0_device::j_UC_x(uint16_t op)
{
	j_xx_x(1);
}
void tms340x0_device::j_P_0(uint16_t op)
{
	j_xx_0(!N_FLAG() && !Z_FLAG());
}
void tms340x0_device::j_P_8(uint16_t op)
{
	j_xx_8(!N_FLAG() && !Z_FLAG());
}
void tms340x0_device::j_P_x(uint16_t op)
{
	j_xx_x(!N_FLAG() && !Z_FLAG());
}
void tms340x0_device::j_LS_0(uint16_t op)
{
	j_xx_0(C_FLAG() || Z_FLAG());
}
void tms340x0_device::j_LS_8(uint16_t op)
{
	j_xx_8(C_FLAG() || Z_FLAG());
}
void tms340x0_device::j_LS_x(uint16_t op)
{
	j_xx_x(C_FLAG() || Z_FLAG());
}
void tms340x0_device::j_HI_0(uint16_t op)
{
	j_xx_0(!C_FLAG() && !Z_FLAG());
}
void tms340x0_device::j_HI_8(uint16_t op)
{
	j_xx_8(!C_FLAG() && !Z_FLAG());
}
void tms340x0_device::j_HI_x(uint16_t op)
{
	j_xx_x(!C_FLAG() && !Z_FLAG());
}
void tms340x0_device::j_LT_0(uint16_t op)
{
	j_xx_0((N_FLAG() && !V_FLAG()) || (!N_FLAG() && V_FLAG()));
}
void tms340x0_device::j_LT_8(uint16_t op)
{
	j_xx_8((N_FLAG() && !V_FLAG()) || (!N_FLAG() && V_FLAG()));
}
void tms340x0_device::j_LT_x(uint16_t op)
{
	j_xx_x((N_FLAG() && !V_FLAG()) || (!N_FLAG() && V_FLAG()));
}
void tms340x0_device::j_GE_0(uint16_t op)
{
	j_xx_0((N_FLAG() && V_FLAG()) || (!N_FLAG() && !V_FLAG()));
}
void tms340x0_device::j_GE_8(uint16_t op)
{
	j_xx_8((N_FLAG() && V_FLAG()) || (!N_FLAG() && !V_FLAG()));
}
void tms340x0_device::j_GE_x(uint16_t op)
{
	j_xx_x((N_FLAG() && V_FLAG()) || (!N_FLAG() && !V_FLAG()));
}
void tms340x0_device::j_LE_0(uint16_t op)
{
	j_xx_0((N_FLAG() && !V_FLAG()) || (!N_FLAG() && V_FLAG()) || Z_FLAG());
}
void tms340x0_device::j_LE_8(uint16_t op)
{
	j_xx_8((N_FLAG() && !V_FLAG()) || (!N_FLAG() && V_FLAG()) || Z_FLAG());
}
void tms340x0_device::j_LE_x(uint16_t op)
{
	j_xx_x((N_FLAG() && !V_FLAG()) || (!N_FLAG() && V_FLAG()) || Z_FLAG());
}
void tms340x0_device::j_GT_0(uint16_t op)
{
	j_xx_0((N_FLAG() && V_FLAG() && !Z_FLAG()) || (!N_FLAG() && !V_FLAG() && !Z_FLAG()));
}
void tms340x0_device::j_GT_8(uint16_t op)
{
	j_xx_8((N_FLAG() && V_FLAG() && !Z_FLAG()) || (!N_FLAG() && !V_FLAG() && !Z_FLAG()));
}
void tms340x0_device::j_GT_x(uint16_t op)
{
	j_xx_x((N_FLAG() && V_FLAG() && !Z_FLAG()) || (!N_FLAG() && !V_FLAG() && !Z_FLAG()));
}
void tms340x0_device::j_C_0(uint16_t op)
{
	j_xx_0(C_FLAG());
}
void tms340x0_device::j_C_8(uint16_t op)
{
	j_xx_8(C_FLAG());
}
void tms340x0_device::j_C_x(uint16_t op)
{
	j_xx_x(C_FLAG());
}
void tms340x0_device::j_NC_0(uint16_t op)
{
	j_xx_0(!C_FLAG());
}
void tms340x0_device::j_NC_8(uint16_t op)
{
	j_xx_8(!C_FLAG());
}
void tms340x0_device::j_NC_x(uint16_t op)
{
	j_xx_x(!C_FLAG());
}
void tms340x0_device::j_EQ_0(uint16_t op)
{
	j_xx_0(Z_FLAG());
}
void tms340x0_device::j_EQ_8(uint16_t op)
{
	j_xx_8(Z_FLAG());
}
void tms340x0_device::j_EQ_x(uint16_t op)
{
	j_xx_x(Z_FLAG());
}
void tms340x0_device::j_NE_0(uint16_t op)
{
	j_xx_0(!Z_FLAG());
}
void tms340x0_device::j_NE_8(uint16_t op)
{
	j_xx_8(!Z_FLAG());
}
void tms340x0_device::j_NE_x(uint16_t op)
{
	j_xx_x(!Z_FLAG());
}
void tms340x0_device::j_V_0(uint16_t op)
{
	j_xx_0(V_FLAG());
}
void tms340x0_device::j_V_8(uint16_t op)
{
	j_xx_8(V_FLAG());
}
void tms340x0_device::j_V_x(uint16_t op)
{
	j_xx_x(V_FLAG());
}
void tms340x0_device::j_NV_0(uint16_t op)
{
	j_xx_0(!V_FLAG());
}
void tms340x0_device::j_NV_8(uint16_t op)
{
	j_xx_8(!V_FLAG());
}
void tms340x0_device::j_NV_x(uint16_t op)
{
	j_xx_x(!V_FLAG());
}
void tms340x0_device::j_N_0(uint16_t op)
{
	j_xx_0(N_FLAG());
}
void tms340x0_device::j_N_8(uint16_t op)
{
	j_xx_8(N_FLAG());
}
void tms340x0_device::j_N_x(uint16_t op)
{
	j_xx_x(N_FLAG());
}
void tms340x0_device::j_NN_0(uint16_t op)
{
	j_xx_0(!N_FLAG());
}
void tms340x0_device::j_NN_8(uint16_t op)
{
	j_xx_8(!N_FLAG());
}
void tms340x0_device::j_NN_x(uint16_t op)
{
	j_xx_x(!N_FLAG());
}

void tms340x0_device::jump_a (uint16_t op) { JUMP(A); }
void tms340x0_device::jump_b (uint16_t op) { JUMP(B); }

void tms340x0_device::popst(uint16_t op)
{
	SET_ST(POP());
	COUNT_CYCLES(8);
}

void tms340x0_device::pushst(uint16_t op)
{
	PUSH(m_st);
	COUNT_CYCLES(2);
}

void tms340x0_device::putst_a (uint16_t op) { PUTST(A); }
void tms340x0_device::putst_b (uint16_t op) { PUTST(B); }

void tms340x0_device::reti(uint16_t op)
{
	int32_t st = POP();
	m_pc = POP();
	CORRECT_ODD_PC("RETI");
	SET_ST(st);
	COUNT_CYCLES(11);
}

void tms340x0_device::rets(uint16_t op)
{
	uint32_t offs;
	m_pc = POP();
	CORRECT_ODD_PC("RETS");
	offs = PARAM_N(op);
	if (offs)
	{
		SP()+=(offs<<4);
	}
	COUNT_CYCLES(7);
}

void tms340x0_device::rev_a (uint16_t op) { REV(A); }
void tms340x0_device::rev_b (uint16_t op) { REV(B); }

void tms340x0_device::trap(uint16_t op)
{
	uint32_t t = PARAM_N(op);
	if (t)
	{
		PUSH(m_pc);
		PUSH(m_st);
	}
	RESET_ST();
	m_pc = RLONG(0xffffffe0-(t<<5));
	CORRECT_ODD_PC("TRAP");
	COUNT_CYCLES(16);
}



/***************************************************************************
    34020 INSTRUCTIONS
***************************************************************************/

/************************************

New 34020 ops:

    0000 1100 000R dddd = ADDXYI IL,Rd
    iiii iiii iiii iiii
    iiii iiii iiii iiii

    0000 0000 1111 00SD = BLMOVE S,D

    0000 0110 0000 0000 = CEXEC S,c,ID,L
    cccc cccc S000 0000
    iiic cccc cccc cccc

    1101 1000 0ccc cccS = CEXEC S,c,ID
    iiic cccc cccc cccc

    0000 1000 1111 0010 = CLIP

    0000 0110 011R dddd = CMOVCG Rd1,Rd2,S,c,ID
    cccc cccc S00R dddd
    iiic cccc cccc cccc

    0000 0110 101R dddd = CMOVCM *Rd+,n,S,c,ID
    cccc cccc S00n nnnn
    iiic cccc cccc cccc

    0000 0110 110R dddd = CMOVCM -*Rd,n,S,c,ID
    cccc cccc S00n nnnn
    iiic cccc cccc cccc

    0000 0110 0110 0000 = CMOVCS c,ID
    cccc cccc 0000 0001
    iiic cccc cccc cccc

    0000 0110 001R ssss = CMOVGC Rs,c,ID
    cccc cccc 0000 0000
    iiic cccc cccc cccc

    0000 0110 010R ssss = CMOVGC Rs1,Rs2,S,c,ID
    cccc cccc S00R ssss
    iiic cccc cccc cccc

    0000 0110 100n nnnn = CMOVMC *Rs+,n,S,c,ID
    cccc cccc S00R ssss
    iiic cccc cccc cccc

    0000 1000 001n nnnn = CMOVMC -*Rs,n,S,c,ID
    cccc cccc S00R ssss
    iiic cccc cccc cccc

    0000 0110 111R dddd = CMOVMC *Rs+,Rd,S,c,ID
    cccc cccc S00R ssss
    iiic cccc cccc cccc

    0011 01kk kkkR dddd = CMPK k,Rd

    0000 1010 100R dddd = CVDXYL Rd

    0000 1010 011R dddd = CVMXYL Rd

    1110 101s sssR dddd = CVSXYL Rs,Rd

    0000 0010 101R dddd = EXGPS Rd

    1101 1110 Z001 1010 = FLINE Z

    0000 1010 1011 1011 = FPIXEQ

    0000 1010 1101 1011 = FPIXNE

    0000 0010 110R dddd = GETPS Rd

    0000 0000 0100 0000 = IDLE

    0000 1100 0101 0111 = LINIT

    0000 0000 1000 0000 = MWAIT

    0000 1010 0011 0111 = PFILL XY

    0000 1110 0001 0111 = PIXBLT L,M,L

    0000 1000 0110 0000 = RETM

    0111 101s sssR dddd = RMO Rs,Rd

    0000 0010 100R dddd = RPIX Rd

    0000 0010 0111 0011 = SETCDP

    0000 0010 1111 1011 = SETCMP

    0000 0010 0101 0001 = SETCSP

    0111 111s sssR dddd = SWAPF *Rs,Rd,0

    0000 1110 1111 1010 = TFILL XY

    0000 1000 0000 1111 = TRAPL

    0000 1000 0101 0111 = VBLT B,L

    0000 1010 0101 0111 = VFILL L

    0000 1010 0000 0000 = VLCOL

************************************/


void tms34020_device::addxyi_a(uint16_t op)
{
	ADD_XYI(A);
}
void tms34020_device::addxyi_b(uint16_t op)
{
	ADD_XYI(B);
}

void tms34020_device::blmove(uint16_t op)
{
	offs_t src = BREG(0);
	offs_t dst = BREG(2);
	offs_t bits = BREG(7);

	bool S = op & (1 << 1);
	bool D = op & (1 << 0);

	if ((S == false && (src & 0xf)) || (D == false && (dst & 0xf))) {
		logerror("020:BLMOVE alignment error: PC=%x: S=%d, D=%d, src=%x, dst=%x, bits=%d\n", m_pc, S, D, src, dst, bits);
	}

	// logerror("020:BLMOVE: PC=%x: S=%d, D=%d, src=%x, dst=%x, bits=%d\n", m_pc, S, D, src, dst, bits);
	while (bits >= 16 && m_icount > 0)
	{
		TMS34010_WRMEM_WORD(dst, TMS34010_RDMEM_WORD(src));
		src += 0x10;
		dst += 0x10;
		bits -= 0x10;
		m_icount -= 2;
	}
	if (bits != 0 && m_icount > 0)
	{
		(this->*s_wfield_functions[bits])(dst, (this->*s_rfield_functions[bits])(src));
		dst += bits;
		src += bits;
		bits = 0;
		m_icount -= 2;
	}

	/*
	    TODO: We do not currently emulate precisely how B0 and B2 are modified during the operation:
	    if D == 0, then B0 and B2 remain fixed during execution and are only incremented after operation completes.
	    if D == 1, then B2 is incremented during move, B0 remains fixed until operation completes.
	*/

	BREG(0) = src;
	BREG(2) = dst;
	BREG(7) = bits;

	// logerror("020:BLMOVE: PC=%x: finished: B0=%x, B2=%x, B7=%d\n", m_pc, src, dst, bits);

	/* if we're not done yet, back up the PC */
	if (bits != 0)
		m_pc -= 0x10;
}

void tms34020_device::cexec_l(uint16_t op)
{
	logerror("020:cexec_l\n");
}

void tms34020_device::cexec_s(uint16_t op)
{
	logerror("020:cexec_s\n");
}

void tms34020_device::clip(uint16_t op)
{
	XY daddr = DADDR_XY();
	XY wstart = WSTART_XY();
	XY wend = WEND_XY();
	XY dydx = DYDX_XY();
	// logerror("020:clip PC=0x%08x: WSTART=(%dx%d) WEND=(%dx%d) DADDR=(%dx%d) DYDX=(%dx%d)\n",
	//      m_pc, wstart.x, wstart.y, wend.x, wend.y, daddr.x, daddr.y, dydx.x, dydx.y);

	// Check whether array intersects with window...
	bool is_l = wstart.x < (daddr.x + dydx.x);
	bool is_r = wend.x > daddr.x;
	bool is_t = wstart.y < (daddr.y + dydx.y);
	bool is_b = wend.y > daddr.y;
	if (!(is_l || is_r || is_t || is_b))
	{
		// ...no intersection, set flags and return
		m_st |= STBIT_Z | STBIT_V;
		// TODO: manual does not specify cycles, only states that this is complex instruction
		COUNT_CYCLES(3);
		return;
	}

	CLR_V();
	CLR_Z();

	// Handle clipping if needed
	bool array_clipped = false;

	if (wstart.x > daddr.x)
	{
		DADDR_X() = wstart.x;
		array_clipped = true;
	}
	if (wend.x < (daddr.x + dydx.x - 1))
	{
		DYDX_X() = wend.x - daddr.x;
		array_clipped = true;
	}

	if (wstart.y > daddr.y)
	{
		DADDR_Y() = wstart.y;
		array_clipped = true;
	}
	if (wend.y < (daddr.y + dydx.y - 1))
	{
		DYDX_Y() = wend.y - daddr.y;
		array_clipped = true;
	}

	if (array_clipped)
		m_st |= STBIT_V;

	// TODO: manual does not specify cycles, only states that this is complex instruction
	COUNT_CYCLES(3);
}

void tms34020_device::cmovcg_a(uint16_t op)
{
	logerror("020:cmovcg_a\n");
}

void tms34020_device::cmovcg_b(uint16_t op)
{
	logerror("020:cmovcg_b\n");
}

void tms34020_device::cmovcm_f(uint16_t op)
{
	logerror("020:cmovcm_f\n");
}

void tms34020_device::cmovcm_b(uint16_t op)
{
	logerror("020:cmovcm_b\n");
}

void tms34020_device::cmovgc_a(uint16_t op)
{
	logerror("020:cmovgc_a\n");
}

void tms34020_device::cmovgc_b(uint16_t op)
{
	logerror("020:cmovgc_b\n");
}

void tms34020_device::cmovgc_a_s(uint16_t op)
{
	logerror("020:cmovgc_a_s\n");
}

void tms34020_device::cmovgc_b_s(uint16_t op)
{
	logerror("020:cmovgc_b_s\n");
}

void tms34020_device::cmovmc_f(uint16_t op)
{
	logerror("020:cmovmc_f\n");
}

void tms34020_device::cmovmc_f_va(uint16_t op)
{
	logerror("020:cmovmc_f_va\n");
}

void tms34020_device::cmovmc_f_vb(uint16_t op)
{
	logerror("020:cmovmc_f_vb\n");
}

void tms34020_device::cmovmc_b(uint16_t op)
{
	logerror("020:cmovmc_b\n");
}

void tms34020_device::cmp_k_a(uint16_t op)
{
	CMPK(A);
}
void tms34020_device::cmp_k_b(uint16_t op)
{
	CMPK(B);
}

void tms34020_device::cvdxyl_a(uint16_t op)
{
	logerror("020:cvdxyl_a\n");
}

void tms34020_device::cvdxyl_b(uint16_t op)
{
	// Convert destination XY address to linear
	// (Y half of Rd x DPTCH) + (X half of Rd x PSIZE) + (A4 or B4) ->  Rd
	const XY rd = BREG_XY(DSTREG(op));
	const off_t dptch = DPTCH();
	const off_t offset = BREG(4);

	const off_t result = (rd.y * dptch) + (rd.x << m_pixelshift) + offset;
	// logerror("020:cvdxyl_b: PC=0x%08x: DADDR=%x, x=%x, y=%x, DPTCH=%d, PIXELSIZE=%d, OFFSET=%d, result: %x\n",
	// m_pc, rd, rd.x, rd.y, dptch, m_pixelshift, offset, result);

	DADDR() = result;

	// Handle cycles related to pitch:
	switch (std::popcount(uint32_t(dptch)))
	{
		// power of 2: 2 clocks
		case 1:
			COUNT_CYCLES(2);
			break;
		// 2 powers of 2: 3 clocks
		case 2:
			COUNT_CYCLES(3);
			break;
		// arbitrary: 14 clocks
		default:
			COUNT_CYCLES(14);
			break;
	}
}

void tms34020_device::cvmxyl_a(uint16_t op)
{
	logerror("020:cvmxyl_a\n");
}

void tms34020_device::cvmxyl_b(uint16_t op)
{
	logerror("020:cvmxyl_b\n");
}

void tms34020_device::cvsxyl_a(uint16_t op)
{
	logerror("020:cvsxyl_a\n");
}

void tms34020_device::cvsxyl_b(uint16_t op)
{
	logerror("020:cvsxyl_b\n");
}

void tms34020_device::exgps_a(uint16_t op)
{
	logerror("020:exgps_a\n");
}

void tms34020_device::exgps_b(uint16_t op)
{
	logerror("020:exgps_b\n");
}

void tms34020_device::fline(uint16_t op)
{
	logerror("020:fline\n");
}

void tms34020_device::fpixeq(uint16_t op)
{
	logerror("020:fpixeq\n");
}

void tms34020_device::fpixne(uint16_t op)
{
	logerror("020:fpixne\n");
}

void tms34020_device::getps_a(uint16_t op)
{
	logerror("020:getps_a\n");
}

void tms34020_device::getps_b(uint16_t op)
{
	logerror("020:getps_b\n");
}

void tms34020_device::idle(uint16_t op)
{
	logerror("020:idle\n");
}

void tms34020_device::linit(uint16_t op)
{
	logerror("020:linit\n");
}

void tms34020_device::mwait(uint16_t op)
{
}

void tms34020_device::pfill_xy(uint16_t op)
{
	logerror("020:pfill_xy\n");
}

void tms34020_device::pixblt_l_m_l(uint16_t op)
{
	logerror("020:pixblt_l_m_l\n");
}

void tms34020_device::retm(uint16_t op)
{
	logerror("020:retm\n");
}


void tms34020_device::rmo_a(uint16_t op) { RMO(A); }
void tms34020_device::rmo_b(uint16_t op) { RMO(B); }


void tms34020_device::rpix_a(uint16_t op)
{
	RPIX(A);
}

void tms34020_device::rpix_b(uint16_t op)
{
	RPIX(B);
}

void tms34020_device::setcdp(uint16_t op)
{
	off_t dptch = DPTCH();

	// Check whether we're dealing with an even number
	if ((dptch & 1) == 0)
	{
		switch (std::popcount(uint32_t(dptch)))
		{
			// .. only single bit set, pitch is power of two!
			case 1:
			{
				m_convdp = std::bit_width(uint32_t(dptch));
				COUNT_CYCLES(4);
				return;
			}
			// .. two bits, we can decompose it to sum of two power of two numbers
			case 2:
			{
				uint16_t v1 = std::bit_width(uint32_t(dptch));
				uint16_t v2 = std::bit_width(uint32_t(dptch) & ~(uint32_t(1) << (v1 - 1)));

				m_convdp = v2 | (v1 << 8);
				COUNT_CYCLES(6);
				return;
			}
		}
	}
	// Default to arbitrary number, setting pitch to 0
	m_convdp = 0;
	COUNT_CYCLES(3);
}

void tms34020_device::setcmp(uint16_t op)
{
	logerror("020:setcmp\n");
}

void tms34020_device::setcsp(uint16_t op)
{
	logerror("020:setcsp\n");
}

void tms34020_device::swapf_a(uint16_t op)
{
	logerror("020:swapf_a\n");
}

void tms34020_device::swapf_b(uint16_t op)
{
	logerror("020:swapf_b\n");
}

void tms34020_device::tfill_xy(uint16_t op)
{
	logerror("020:tfill_xy\n");
}

void tms34020_device::trapl(uint16_t op)
{
	logerror("020:trapl\n");
}

void tms34020_device::vblt_b_l(uint16_t op)
{
	// Linear VRAM pixel block transfer
	// logerror("020:vblt_b_l: SADDR=0x%x, SPTCH=%d, DADDR=0x%x, DPTCH=%d, DYDX=(%dx%d), TEMP=%x\n", SADDR(), SPTCH(), DADDR(), DPTCH(), DYDX_X(), DYDX_Y(), TEMP());
	logerror("020:vblt_b_l\n");
}

void tms34020_device::vfill_l(uint16_t op)
{
	// Linear VRAM fast fill
	// logerror("020:vfill_l: DADDR=0x%x, DPTCH=%d, DYDX=(%dx%d)\n", DADDR(), DPTCH(), DYDX_X(), DYDX_Y());
	logerror("020:vfill_l\n");
}

void tms34020_device::vlcol(uint16_t op)
{
	// Latch COLOR1 into the VRAM color registers
	//logerror("020:vlcol: COLOR1=%x\n", COLOR1());
	logerror("020:vlcol\n");
}
