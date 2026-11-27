// mkrecomp.cpp - static recompiler: TMS34010 program ROM -> C++.
//
//   mkrecomp <folder with roms/> <game id> <output dir> [extra entry points file...]
//   mkrecomp --image <1 MB program image> <output dir>        (test: module id "test")
//
// Reads the main CPU program of the user's own ROMs, finds the code (recursive descent from
// the 32 trap/interrupt vectors, call/jump targets and code pointers loaded by MOVI/CALLA/JA,
// plus the addresses listed in the optional files, e.g. recomp/<game>.miss written by the
// application), and writes C++ in which every instruction is a label:
//   * branches (JRcc / JAcc / DSJ / DSJS) become native C++ conditions and gotos,
//   * simple register instructions use the interpreter's own macros with the opcode as a
//     compile-time constant,
//   * every other instruction is a direct call to the interpreter's handler for it, with the
//     opcode already decoded.
// After each instruction the cycle budget and the PC are checked exactly as in the
// interpreter loop, so the result is identical instruction for instruction; anything the
// recompiled code does not cover (code in RAM, addresses not found) runs in the interpreter.
//
// The output is derived from copyrighted ROMs: it is built on the user's machine and must
// never be committed or distributed.
#include "emu.h"
#include "cpu/tms34010/34010dsm.h"
#include "games.h"
#include "../../src/recomp/rc_abi.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

static const char *const k_names[4096] = {
#include "../../src/recomp/tms34010_opnames.inc"
};

static constexpr u32 BASE = 0xff800000;
static std::vector<u8> g_img;
static u32 g_words;

static bool in_rom(u32 a) { return a >= BASE && !(a & 0xf) && ((a - BASE) >> 4) < g_words; }
static u16 W(u32 a)
{
	u32 w = (a - BASE) >> 4;
	return w < g_words ? u16(g_img[w * 2] | (g_img[w * 2 + 1] << 8)) : 0;
}
static u32 L(u32 a) { return u32(W(a)) | (u32(W(a + 16)) << 16); }

struct insn
{
	u32 addr = 0;
	u16 op = 0;
	u32 len = 16;               // bits
	std::string name;
	enum kind { normal, branch, call, end, bad } k = normal;
	bool falls = true;          // execution can continue at addr + len
	u32 target = 0;             // static branch / call target (0 = none)
	std::string text;           // disassembly
};

static tms34010_disassembler g_dasm(false);

static insn decode(u32 a)
{
	insn i;
	i.addr = a;
	i.op = W(a);
	i.name = k_names[i.op >> 4];
	util::disasm_interface::data_buffer buf;
	buf.reader = [&](offs_t ad, int bits) -> u64 {
		u64 v = 0;
		for (int k = 0; k < bits / 16; k++) v |= u64(W(ad + 16 * k)) << (16 * k);
		return v;
	};
	std::ostringstream ss;
	offs_t r = g_dasm.disassemble(ss, a, buf, buf);
	i.len = r & 0xffff;             // in bits
	if (i.len == 0) i.len = 16;
	i.text = ss.str();
	const std::string &n = i.name;
	u16 op = i.op;
	auto rel8 = [&]() { return u32(a + 16 + (u32(int32_t(int8_t(op & 0xff))) << 4)); };
	if (n == "illop" || n == "unimpl") { i.k = insn::bad; i.falls = false; }
	else if (n.rfind("j_", 0) == 0)
	{
		i.k = insn::branch;
		char form = n.back();
		bool uc = n.compare(0, 4, "j_UC") == 0;
		if (form == 'x' || (op & 0x0f)) { i.len = 16; i.target = rel8(); }
		else if (form == '0') { i.len = 32; i.target = u32(a + 32 + (u32(int32_t(int16_t(W(a + 16)))) << 4)); }
		else { i.len = 48; i.target = L(a + 16) & ~0xfu; }
		i.falls = !uc;
	}
	else if (n == "dsj_a" || n == "dsj_b" || n == "dsjeq_a" || n == "dsjeq_b" || n == "dsjne_a" || n == "dsjne_b")
	{
		i.k = insn::branch; i.len = 32;
		i.target = u32(a + 32 + (u32(int32_t(int16_t(W(a + 16)))) << 4));
	}
	else if (n == "dsjs_a" || n == "dsjs_b")
	{
		i.k = insn::branch; i.len = 16;
		u32 k = (op >> 5) & 0x1f;
		i.target = (op & 0x0400) ? u32(a + 16 - (k << 4)) : u32(a + 16 + (k << 4));
	}
	else if (n == "callr") { i.k = insn::call; i.len = 32; i.target = u32(a + 32 + (u32(int32_t(int16_t(W(a + 16)))) << 4)); }
	else if (n == "calla") { i.k = insn::call; i.len = 48; i.target = L(a + 16) & ~0xfu; }
	else if (n == "rets" || n == "reti" || n == "jump_a" || n == "jump_b")
	{ i.k = insn::end; i.falls = false; }
	// EXGPC Rn swaps PC and Rn: the games use it as a call (the callee comes back with
	// another EXGPC), so the next instruction is reachable
	else if (n == "exgpc_a" || n == "exgpc_b") i.k = insn::call;
	return i;
}

// a plausible start of code: a few instructions without illegal opcodes, inside the ROM
static bool plausible(u32 a)
{
	for (int n = 0; n < 8; n++)
	{
		if (!in_rom(a)) return false;
		insn i = decode(a);
		if (i.k == insn::bad) return false;
		if (!i.falls) return true;
		a += i.len;
	}
	return true;
}

// ---------------------------------------------------------------------------
// C++ emission
// ---------------------------------------------------------------------------
static const char *cond_expr(const std::string &c)
{
	static const std::map<std::string, const char *> m = {
		{ "UC", "true" }, { "P", "!N_FLAG() && !Z_FLAG()" }, { "LS", "C_FLAG() || Z_FLAG()" }, { "HI", "!C_FLAG() && !Z_FLAG()" },
		{ "LT", "(N_FLAG() && !V_FLAG()) || (!N_FLAG() && V_FLAG())" }, { "GE", "(N_FLAG() && V_FLAG()) || (!N_FLAG() && !V_FLAG())" },
		{ "LE", "(N_FLAG() && !V_FLAG()) || (!N_FLAG() && V_FLAG()) || Z_FLAG()" },
		{ "GT", "(N_FLAG() && V_FLAG() && !Z_FLAG()) || (!N_FLAG() && !V_FLAG() && !Z_FLAG())" },
		{ "C", "C_FLAG()" }, { "NC", "!C_FLAG()" }, { "EQ", "Z_FLAG()" }, { "NE", "!Z_FLAG()" },
		{ "V", "V_FLAG()" }, { "NV", "!V_FLAG()" }, { "N", "N_FLAG()" }, { "NN", "!N_FLAG()" } };
	auto it = m.find(c);
	return it == m.end() ? nullptr : it->second;
}

// register-only instructions emitted with the interpreter's macros (no immediates, PC untouched)
static const std::map<std::string, std::string> &native_macros()
{
	static const std::map<std::string, std::string> m = {
		{ "move_rr_a", "MOVE_RR(A,A)" }, { "move_rr_b", "MOVE_RR(B,B)" }, { "move_rr_ax", "MOVE_RR(A,B)" }, { "move_rr_bx", "MOVE_RR(B,A)" },
		{ "cmp_a", "CMP(A)" }, { "cmp_b", "CMP(B)" }, { "add_a", "ADD(A)" }, { "add_b", "ADD(B)" }, { "sub_a", "SUB(A)" }, { "sub_b", "SUB(B)" },
		{ "and_a", "AND(A)" }, { "and_b", "AND(B)" }, { "or_a", "OR(A)" }, { "or_b", "OR(B)" }, { "xor_a", "XOR(A)" }, { "xor_b", "XOR(B)" },
		{ "addk_a", "ADDK(A)" }, { "addk_b", "ADDK(B)" }, { "subk_a", "SUBK(A)" }, { "subk_b", "SUBK(B)" },
		{ "movk_a", "MOVK(A)" }, { "movk_b", "MOVK(B)" }, { "btst_k_a", "BTST_K(A)" }, { "btst_k_b", "BTST_K(B)" },
		{ "srl_k_a", "SRL(A,PARAM_K(op))" }, { "srl_k_b", "SRL(B,PARAM_K(op))" }, { "sll_k_a", "SLL(A,PARAM_K(op))" }, { "sll_k_b", "SLL(B,PARAM_K(op))" },
		{ "sra_k_a", "SRA(A,PARAM_K(op))" }, { "sra_k_b", "SRA(B,PARAM_K(op))" }, { "rl_k_a", "RL(A,PARAM_K(op))" }, { "rl_k_b", "RL(B,PARAM_K(op))" },
		{ "rl_r_a", "RL(A,AREG(SRCREG(op))&0x1f)" }, { "rl_r_b", "RL(B,BREG(SRCREG(op))&0x1f)" },
		{ "add_xy_a", "ADD_XY(A)" }, { "add_xy_b", "ADD_XY(B)" }, { "sub_xy_a", "SUB_XY(A)" }, { "sub_xy_b", "SUB_XY(B)" },
		{ "cmp_xy_a", "CMP_XY(A)" }, { "cmp_xy_b", "CMP_XY(B)" }, { "movx_a", "MOVX(A)" }, { "movx_b", "MOVX(B)" },
		{ "movy_a", "MOVY(A)" }, { "movy_b", "MOVY(B)" },
	};
	return m;
}

int main(int argc, char **argv)
{
	if (argc < 4)
	{
		std::fprintf(stderr, "usage: mkrecomp <folder with roms/> <game: mk mk2 mk3 umk3> <output dir> [entry point files...]\n");
		return 2;
	}
	std::string base = argv[1], id = argv[2], out = argv[3];
	if (base == "--image")
	{
		// test mode: a raw 1 MB program image (CI builds a module from synthetic data)
		std::ifstream in(id, std::ios::binary);
		g_img.assign(std::istreambuf_iterator<char>(in), {});
		g_img.resize(0x100000, 0);
		id = "test";
	}
	else
	{
		game_catalog cat(base);
		const game_info *gi = nullptr;
		for (auto &g : game_list()) if (id == g.id) gi = &g;
		if (!gi) { std::fprintf(stderr, "unknown game %s\n", id.c_str()); return 2; }
		std::string err;
		if (!cat.program_image(*gi, g_img, err)) { std::fprintf(stderr, "%s: %s\n", id.c_str(), err.c_str()); return 1; }
	}
	g_words = u32(g_img.size() / 2);
	if (const char *ls = getenv("MKRC_LIST"))   // debugging: MKRC_LIST=start,end lists the disassembly
	{
		u32 a = u32(strtoul(ls, nullptr, 16)), e = u32(strtoul(strchr(ls, ',') + 1, nullptr, 16));
		while (a < e) { insn i = decode(a); std::printf("%08x %04x %-12s %s\n", a, i.op, i.name.c_str(), i.text.c_str()); a += i.len; }
		return 0;
	}
	u64 rom_hash = rc_fnv64(g_img.data(), g_img.size());

	// --- discovery
	std::map<u32, insn> code;
	std::vector<u32> work;
	std::set<u32> roots;
	auto add_root = [&](u32 a, bool check) {
		if (!in_rom(a) || code.count(a) || roots.count(a)) return;
		if (check && !plausible(a)) return;
		roots.insert(a);
		work.push_back(a);
	};
	for (int n = 0; n < 32; n++) add_root(L(0xffffffe0u - 0x20u * n) & ~0xfu, true);
	// code pointers stored anywhere in the ROM (process tables, jump tables, script data):
	// every 32-bit value at a word boundary that points into the program and starts with a
	// believable run of instructions
	int scanned = 0;
	for (u32 w = 0; w + 1 < g_words; w++)
	{
		u32 v = u32(g_img[w * 2] | (g_img[w * 2 + 1] << 8)) | (u32(g_img[w * 2 + 2] | (g_img[w * 2 + 3] << 8)) << 16);
		if (in_rom(v) && !roots.count(v) && plausible(v)) { add_root(v, false); scanned++; }
	}
	int extra = 0;
	for (int f = 4; f < argc; f++)
	{
		std::ifstream in(argv[f]);
		std::string line;
		while (std::getline(in, line))
		{
			u32 a = u32(std::strtoul(line.c_str(), nullptr, 16));
			if (in_rom(a)) { add_root(a, false); extra++; }
		}
	}
	while (!work.empty())
	{
		u32 a = work.back();
		work.pop_back();
		while (in_rom(a) && !code.count(a))
		{
			insn i = decode(a);
			if (i.k == insn::bad) break;
			code[a] = i;
			if (i.target) add_root(i.target, i.k != insn::branch && i.k != insn::call);
			if (i.len == 48 && i.k == insn::normal) add_root(L(a + 16) & ~0xfu, true);   // MOVI/MOVE etc. with a code pointer
			if (!i.falls) break;
			a += i.len;
		}
	}
	std::fprintf(stderr, "%s: %zu instructions, %zu entry points (%d pointers found in the ROM, %d from files)\n", id.c_str(), code.size(), roots.size(), scanned, extra);

	// --- regions: runs of instructions where each one follows the previous
	std::vector<std::vector<u32>> regions;
	{
		std::vector<u32> cur;
		u32 expect = 0;
		for (auto &[a, i] : code)
		{
			if (!cur.empty() && (a != expect || cur.size() >= 3000)) { regions.push_back(cur); cur.clear(); }
			cur.push_back(a);
			expect = a + i.len;
		}
		if (!cur.empty()) regions.push_back(cur);
	}

	// --- output
	const auto &nat = native_macros();
	size_t per_file = 40;
	size_t nfiles = (regions.size() + per_file - 1) / per_file;
	std::string tag = "rc_" + id;
	{
		std::ofstream h(out + "/" + tag + ".h");
		h << "// generated by mkrecomp from the user's own ROM - do not distribute\n#pragma once\n"
		     "#include \"rc_module.h\"\n\n";
		for (size_t r = 0; r < regions.size(); r++) h << "int " << tag << "_rgn_" << r << "(const rc_host *h, uint32_t pc);\n";
	}
	size_t native = 0, total = 0;
	for (size_t f = 0; f < nfiles; f++)
	{
		std::ofstream c(out + "/" + tag + "_" + std::to_string(f) + ".cpp");
		c << "// generated by mkrecomp from the user's own ROM - do not distribute\n"
		     "#include \"" << tag << ".h\"\n"
		     "#pragma GCC diagnostic ignored \"-Wunused-label\"\n\n";
		for (size_t r = f * per_file; r < std::min(regions.size(), (f + 1) * per_file); r++)
		{
			const auto &rg = regions[r];
			std::set<u32> in_rgn(rg.begin(), rg.end());
			c << "int " << tag << "_rgn_" << r << "(const rc_host *h, uint32_t pc)\n{\n\tRC_BIND(h);\n\tint ran = 0;\ndispatch:\n\tswitch (pc)\n\t{\n";
			for (u32 a : rg) c << "\tcase 0x" << std::hex << a << "u: goto L" << a << ";\n" << std::dec;
			c << "\tdefault: return ran;\n\t}\n";
			auto go = [&](u32 t) -> std::string {
				char b[64];
				if (in_rgn.count(t)) std::snprintf(b, sizeof b, "goto L%x;", t);
				else std::snprintf(b, sizeof b, "return 1;");
				return b;
			};
			for (size_t k = 0; k < rg.size(); k++)
			{
				const insn &i = code[rg[k]];
				u32 a = i.addr, n = a + i.len;
				bool next_is_n = k + 1 < rg.size() && rg[k + 1] == n;
				char hb[160];
				std::string text = i.text;
				for (char &ch : text) if (ch == '\n' || ch == '\r') ch = ' ';
				std::snprintf(hb, sizeof hb, "L%x: // %s\n\tran = 1; m_ppc = 0x%xu;\n", a, text.c_str(), a);
				c << hb;
				total++;
				const std::string &nm = i.name;
				std::string fall = next_is_n ? "" : "\t" + go(n) + "\n";
				if (nm.rfind("j_", 0) == 0 && cond_expr(nm.substr(2, nm.size() - 4)))
				{
					native++;
					std::string cond = cond_expr(nm.substr(2, nm.size() - 4));
					char form = nm.back();
					int ct, cn;
					if (form == 'x' || (i.op & 0x0f)) { ct = 2; cn = 1; }
					else if (form == '0') { ct = 3; cn = 2; }
					else { ct = 3; cn = 4; }
					std::snprintf(hb, sizeof hb, "\tif (%s) { m_icount -= %d; m_pc = 0x%xu; if (m_icount <= 0) return 1; ", cond.c_str(), ct, i.target);
					c << hb << go(i.target) << " }\n";
					if (i.falls)
					{
						std::snprintf(hb, sizeof hb, "\tm_icount -= %d; m_pc = 0x%xu; if (m_icount <= 0) return 1;\n", cn, n);
						c << hb << fall;
					}
					continue;
				}
				if (nm == "dsjs_a" || nm == "dsjs_b" || nm == "dsj_a" || nm == "dsj_b" ||
				    nm == "dsjeq_a" || nm == "dsjeq_b" || nm == "dsjne_a" || nm == "dsjne_b")
				{
					native++;
					char R = nm.back() == 'a' ? 'A' : 'B';
					bool s = nm.rfind("dsjs", 0) == 0;
					std::string pre = nm.rfind("dsjeq", 0) == 0 ? "Z_FLAG() && " : nm.rfind("dsjne", 0) == 0 ? "!Z_FLAG() && " : "";
					int ct = s ? 2 : 3, cn = s ? 3 : 2;
					// the register is only decremented when the Z condition holds
					std::snprintf(hb, sizeof hb, "\tif (%s--%cREG(%d)) { m_icount -= %d; m_pc = 0x%xu; if (m_icount <= 0) return 1; ",
					              pre.c_str(), R, i.op & 0xf, ct, i.target);
					c << hb << go(i.target) << " }\n";
					std::snprintf(hb, sizeof hb, "\tm_icount -= %d; m_pc = 0x%xu; if (m_icount <= 0) return 1;\n", cn, n);
					c << hb << fall;
					continue;
				}
				auto it = nat.find(nm);
				if (it != nat.end())
				{
					native++;
					std::snprintf(hb, sizeof hb, "\t{ const uint16_t op = 0x%04x; %s } if (m_icount <= 0) { m_pc = 0x%xu; return 1; }\n", i.op, it->second.c_str(), n);
					c << hb;
					if (!next_is_n) { std::snprintf(hb, sizeof hb, "\tm_pc = 0x%xu; ", n); c << hb << go(n) << "\n"; }
					continue;
				}
				// any other instruction: the interpreter's handler, called directly
				std::snprintf(hb, sizeof hb, "\tm_pc = 0x%xu; rc_exec(rc_cpu, 0x%04x); // %s\n\tif (m_icount <= 0) return 1;\n", a + 16, i.op, nm.c_str());
				c << hb;
				if (i.falls)
				{
					std::snprintf(hb, sizeof hb, "\tif (m_pc != 0x%xu) { pc = m_pc; goto dispatch; }\n", n);
					c << hb << fall;
				}
				else c << "\tpc = m_pc; goto dispatch;\n";
			}
			c << "}\n\n";
		}
	}
	{
		std::ofstream m(out + "/" + tag + "_main.cpp");
		m << "// generated by mkrecomp from the user's own ROM - do not distribute\n#include \"" << tag << ".h\"\n\n"
		     "typedef int (*rgn_fn)(const rc_host *, uint32_t);\nstatic const rgn_fn k_rgn[] = {\n";
		for (size_t r = 0; r < regions.size(); r++) m << "\t" << tag << "_rgn_" << r << ",\n";
		m << "};\n\n// region (+1) of every instruction address, per 16-bit word of the program ROM\n";
		std::vector<u32> map(g_words, 0);
		for (size_t r = 0; r < regions.size(); r++)
			for (u32 a : regions[r]) map[(a - BASE) >> 4] = u32(r + 1);
		m << "static const uint16_t k_map[" << g_words << "] = {";
		for (u32 w = 0; w < g_words; w++) { if (!(w % 32)) m << "\n"; m << map[w] << ","; }
		char hb[256];
		std::snprintf(hb, sizeof hb, "\n};\n\nstatic int rc_run(const rc_host *h)\n{\n"
		              "\tuint32_t pc = *h->pc;\n\tif (pc < 0x%xu || (pc & 0xf)) return 0;\n"
		              "\tuint32_t w = (pc - 0x%xu) >> 4;\n\tif (w >= %uu || !k_map[w]) return 0;\n"
		              "\treturn k_rgn[k_map[w] - 1](h, pc);\n}\n\n", BASE, BASE, g_words);
		m << hb;
		std::snprintf(hb, sizeof hb, "static const rc_module k_module = { RC_ABI_VERSION, 0x%llxull, \"%s\", 0x%xu, %uu, %zuu, rc_run };\n",
		              (unsigned long long)rom_hash, id.c_str(), BASE, g_words, total);
		m << hb << "\nRC_EXPORT const rc_module *mkrc_get_module(void) { return &k_module; }\n";
	}
	{
		std::ofstream l(out + "/" + tag + ".files");
		for (size_t f = 0; f < nfiles; f++) l << tag << "_" << f << ".cpp\n";
		l << tag << "_main.cpp\n";
	}
	std::fprintf(stderr, "%s: %zu regions, %zu files, %zu of %zu instructions as native C++ (rest: direct handler calls)\n",
	             id.c_str(), regions.size(), nfiles + 1, native, total);
	return 0;
}
