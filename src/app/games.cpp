// games.cpp - the Mortal Kombat arcade saga: game list, where each game's ROM data comes
// from, and construction of the emulated board.
#include "games.h"
#include "lang.h"
#include "tunit.h"
#include "wunit.h"

#include <cstring>

const std::vector<game_info> &game_list()
{
	// ordered by release date
	static const std::vector<game_info> list = {
		{ "mk",     "MORTAL KOMBAT",                  "1992 - rev 5.0 / 4.0 T-Unit (romset MAME mk)",     "1992 - rev 5.0 / 4.0 T-Unit (MAME romset mk)",     "T-Unit + ADPCM", false },
		{ "mk2",    "MORTAL KOMBAT II",               "1993 - rev L3.1 (romset MAME mk2)",                "1993 - rev L3.1 (MAME romset mk2)",                "T-Unit + DCS",   false },
		{ "mk3",    "MORTAL KOMBAT 3",                "1995 - rev 1.0 / 2.0 / 2.1 (romset MAME mk3r10)",  "1995 - rev 1.0 / 2.0 / 2.1 (MAME romset mk3r10)",  "Wolf Unit",      false },
		{ "umk3",   "ULTIMATE MORTAL KOMBAT 3",       "1995 - rev 1.2 / 1.1 / 1.0 (romset MAME umk3)",    "1995 - rev 1.2 / 1.1 / 1.0 (MAME romset umk3)",    "Wolf Unit",      false },
	};
	return list;
}

// ---------------------------------------------------------------------------
// ROM descriptions (names / CRCs from MAME midtunit.cpp, midwunit.cpp). Files are matched
// by CRC32 only, so any file name, folder or ZIP works.
// ---------------------------------------------------------------------------
struct rom_desc { const char *name; uint32_t crc; };
struct prog_version { const char *label; rom_desc lo, hi; };

// --- Mortal Kombat (T-Unit, Williams ADPCM sound) ---
static const prog_version mk_versions[] = {
	{ "5.0", { "mkt-uj12.bin (rev 5.0)", 0xf4990bf2 }, { "mkt-ug12.bin (rev 5.0)", 0xb06aeac1 } },
	{ "4.0", { "uj12 (rev 4.0)", 0xa1b6635a }, { "ug12 (rev 4.0)", 0xaa94f7ea } },
};
static const rom_desc mk_snd[3] = { { "mks-u3.rom", 0xc615844c }, { "mks-u12.rom", 0x258bd7f9 }, { "mks-u13.rom", 0x7b7ec3b6 } };
static const rom_desc mk_gfx[12] = {
	{ "mkt-ug14.bin", 0x9e00834e }, { "mkt-uj14.bin", 0xf4b0aaa7 }, { "mkt-ug19.bin", 0x2d8c7ba1 }, { "mkt-uj19.bin", 0x33b9b7a4 },
	{ "mkt-ug16.bin", 0x52c9d1e5 }, { "mkt-uj16.bin", 0xc94c58cf }, { "mkt-ug20.bin", 0x2f7e55d3 }, { "mkt-uj20.bin", 0xeae96df0 },
	{ "mkt-ug17.bin", 0xe34fe253 }, { "mkt-uj17.bin", 0xa56e12f5 }, { "mkt-ug22.bin", 0xb537bb4e }, { "mkt-uj22.bin", 0x5e12523b } };

// --- Mortal Kombat II (T-Unit, DCS 2K sound) ---
static const prog_version mk2_versions[] = {
	{ "L3.1", { "uj12.l31", 0xcf100a75 }, { "ug12.l31", 0x582c7dfd } },
};
static const rom_desc mk2_dcs[6] = {
	{ "su2.l1", 0x5f23d71d }, { "su3.l1", 0xd6d92bf9 }, { "su4.l1", 0xeebc8e0f },
	{ "su5.l1", 0x2b0b7961 }, { "su6.l1", 0xf694b27f }, { "su7.l1", 0x20387e0a } };
static const rom_desc mk2_gfx[12] = {
	{ "ug14-vid", 0x01e73af6 }, { "uj14-vid", 0xd4985cbb }, { "ug19-vid", 0xfec137be }, { "uj19-vid", 0x2d763156 },
	{ "ug16-vid", 0x8ba6ae18 }, { "uj16-vid", 0x39d885b4 }, { "ug20-vid", 0x809118c1 }, { "uj20-vid", 0xb96824f0 },
	{ "ug17-vid", 0x937d8620 }, { "uj17-vid", 0x218de160 }, { "ug22-vid", 0x154d53b1 }, { "uj22-vid", 0x8891d785 } };

// --- Mortal Kombat 3 / UMK3 (Wolf Unit, DCS 8K sound) ---
static const prog_version mk3_versions[] = {
	{ "1.0", { "mk310u54.bin", 0x41829228 }, { "mk310u63.bin", 0xb074e1e8 } },
	{ "2.1", { "mk321u54.bin", 0x9e344401 }, { "mk321u63.bin", 0x64d34776 } },
	{ "2.0", { "mk320u54.bin", 0x453da302 }, { "mk320u63.bin", 0xf8dc0600 } },
};
static const prog_version umk3_versions[] = {
	{ "1.2", { "um312u54.bin", 0x712b4db6 }, { "um312u63.bin", 0x6d301faf } },
	{ "1.1", { "l1.1 u54", 0x8bb27659 }, { "l1.1 u63", 0xea731783 } },
	{ "1.0", { "l1.0 u54", 0xdfd735da }, { "l1.0 u63", 0x2dff0c83 } },
};
static const rom_desc mk3_dcs[4] = { { "mk3-u2.bin", 0x5273436f }, { "mk3-u3.bin", 0x856fe411 }, { "mk3-u4.bin", 0x428a406f }, { "mk3-u5.bin", 0x3b98a09f } };
static const rom_desc umk3_dcs[4] = { { "umk3-u2.bin", 0x3838cfe5 }, { "mk3-u3.bin", 0x856fe411 }, { "mk3-u4.bin", 0x428a406f }, { "mk3-u5.bin", 0x3b98a09f } };
struct gfx_desc { rom_desc r; uint32_t off; };
static const gfx_desc mk3_gfx[20] = {
	{ { "mk3-u133.bin", 0x79b94667 }, 0x0000000 }, { { "mk3-u132.bin", 0x13e95228 }, 0x0000001 }, { { "mk3-u131.bin", 0x41001e30 }, 0x0000002 }, { { "mk3-u130.bin", 0x49379dd7 }, 0x0000003 },
	{ { "mk3-u129.bin", 0xa8b41803 }, 0x0400000 }, { { "mk3-u128.bin", 0xb410d72f }, 0x0400001 }, { { "mk3-u127.bin", 0xbd985be7 }, 0x0400002 }, { { "mk3-u126.bin", 0xe7c32cf4 }, 0x0400003 },
	{ { "mk3-u125.bin", 0x9a52227e }, 0x0800000 }, { { "mk3-u124.bin", 0x5c750ebc }, 0x0800001 }, { { "mk3-u123.bin", 0xf0ab88a8 }, 0x0800002 }, { { "mk3-u122.bin", 0x9b87cdac }, 0x0800003 },
	{ { "mk3-u121.bin", 0xb6c6296a }, 0x0c00000 }, { { "mk3-u120.bin", 0x8d1ccc3b }, 0x0c00001 }, { { "mk3-u119.bin", 0x63215b59 }, 0x0c00002 }, { { "mk3-u118.bin", 0x8b681e34 }, 0x0c00003 },
	{ { "mk3-u117.bin", 0x1ab20377 }, 0x1000000 }, { { "mk3-u116.bin", 0xba246ad0 }, 0x1000001 }, { { "mk3-u115.bin", 0x3ee8b124 }, 0x1000002 }, { { "mk3-u114.bin", 0xa8d99922 }, 0x1000003 } };
static const gfx_desc umk3_gfx[20] = {
	{ { "mk3-u133.bin", 0x79b94667 }, 0x0000000 }, { { "mk3-u132.bin", 0x13e95228 }, 0x0000001 }, { { "mk3-u131.bin", 0x41001e30 }, 0x0000002 }, { { "mk3-u130.bin", 0x49379dd7 }, 0x0000003 },
	{ { "mk3-u129.bin", 0xa8b41803 }, 0x0400000 }, { { "mk3-u128.bin", 0xb410d72f }, 0x0400001 }, { { "mk3-u127.bin", 0xbd985be7 }, 0x0400002 }, { { "mk3-u126.bin", 0xe7c32cf4 }, 0x0400003 },
	{ { "mk3-u125.bin", 0x9a52227e }, 0x0800000 }, { { "mk3-u124.bin", 0x5c750ebc }, 0x0800001 }, { { "mk3-u123.bin", 0xf0ab88a8 }, 0x0800002 }, { { "mk3-u122.bin", 0x9b87cdac }, 0x0800003 },
	{ { "umk-u121.bin", 0xcc4b95db }, 0x0c00000 }, { { "umk-u120.bin", 0x1c8144cd }, 0x0c00001 }, { { "umk-u119.bin", 0x5f10c543 }, 0x0c00002 }, { { "umk-u118.bin", 0xde0c4488 }, 0x0c00003 },
	{ { "umk-u113.bin", 0x99d74a1e }, 0x1400000 }, { { "umk-u112.bin", 0xb5a46488 }, 0x1400001 }, { { "umk-u111.bin", 0xa87523c8 }, 0x1400002 }, { { "umk-u110.bin", 0x0038f205 }, 0x1400003 } };

// ---------------------------------------------------------------------------
game_catalog::game_catalog(const std::string &base_dir) : m_base(base_dir) {}

rom_pool &game_catalog::pool()
{
	if (!m_pool_scanned)
	{
		m_pool.add_tree(path_join(m_base, "roms"));
		m_pool_scanned = true;
	}
	return m_pool;
}

std::string game_catalog::data_dir(const game_info &g) const
{
	return path_join(path_join(m_base, "data"), g.id);
}

bool game_catalog::uses_data_dir(const game_info &) const { return false; }

static bool have(rom_pool &p, const rom_desc &d) { return p.has("", d.crc); }
static bool have_all(rom_pool &p, const rom_desc *d, int n)
{
	for (int i = 0; i < n; i++)
		if (!have(p, d[i])) return false;
	return true;
}
static bool have_gfx(rom_pool &p, const gfx_desc *g, int n)
{
	for (int i = 0; i < n; i++)
		if (!have(p, g[i].r)) return false;
	return true;
}
static bool load_desc(rom_pool &p, const rom_desc &d, std::vector<u8> &out, std::string &err)
{
	if (!p.get("", d.crc, out)) { err = std::string(tr("no se encuentra ", "missing ")) + d.name; return false; }
	return true;
}
template <size_t N>
static const prog_version *find_version(rom_pool &p, const prog_version (&v)[N])
{
	for (auto &x : v)
		if (have(p, x.lo) && have(p, x.hi)) return &x;
	return nullptr;
}

bool game_catalog::available(const game_info &g, std::string &why)
{
	return from_roms(g, why);
}

bool game_catalog::from_roms(const game_info &g, std::string &why)
{
	std::string id = g.id;
	rom_pool &p = pool();
	bool ok = false;
	if (id == "mk") ok = find_version(p, mk_versions) && have_all(p, mk_gfx, 12) && have_all(p, mk_snd, 3);
	else if (id == "mk2") ok = find_version(p, mk2_versions) && have_all(p, mk2_gfx, 12) && have_all(p, mk2_dcs, 6);
	else if (id == "mk3") ok = find_version(p, mk3_versions) && have_gfx(p, mk3_gfx, 20) && have_all(p, mk3_dcs, 4);
	else if (id == "umk3") ok = find_version(p, umk3_versions) && have_gfx(p, umk3_gfx, 20) && have_all(p, umk3_dcs, 4);
	else { why = tr("juego desconocido", "unknown game"); return false; }
	if (ok) return true;
	const char *zip = id == "mk" ? "mk.zip" : id == "mk2" ? "mk2.zip" : id == "mk3" ? "mk3r10.zip / mk3.zip" : "umk3.zip";
	why = std::string(tr("Copia el romset MAME ", "Copy the MAME romset ")) + zip + tr(" en la carpeta roms/", " to the roms/ folder");
	return false;
}

static void interleave(const std::vector<u8> &lo, const std::vector<u8> &hi, std::vector<u8> &out)
{
	out.assign(0x100000, 0xff);
	for (size_t i = 0; i < 0x80000 && i < lo.size() && i < hi.size(); i++) { out[i * 2] = lo[i]; out[i * 2 + 1] = hi[i]; }
}

// T-unit graphics: banks of 4 byte lanes (ROM_LOAD32_BYTE)
static bool load_tunit_lanes(rom_pool &p, const rom_desc *gfx, int banks, size_t rom_size, std::vector<u8> &out, std::string &err)
{
	std::vector<u8> d;
	out.assign(0xc00000, 0);
	for (int b = 0; b < banks; b++)
		for (int l = 0; l < 4; l++)
		{
			if (!load_desc(p, gfx[b * 4 + l], d, err)) return false;
			for (size_t k = 0; k < rom_size && k < d.size(); k++) out[b * rom_size * 4 + k * 4 + l] = d[k];
		}
	return true;
}

static bool load_wolf_gfx(rom_pool &p, const gfx_desc *g, int n, std::vector<u8> &gfx, std::string &err)
{
	std::vector<u8> d;
	gfx.assign(0x2000000, 0);
	for (int i = 0; i < n; i++)
	{
		if (!load_desc(p, g[i].r, d, err)) return false;
		u32 base = g[i].off & ~3u, lane = g[i].off & 3u;
		for (size_t k = 0; k < d.size() && k < 0x100000; k++) gfx[base + k * 4 + lane] = d[k];
	}
	return true;
}

// DCS sound data: ROM_LOAD16_BYTE into a 16-bit region -> one ROM byte per word.
// 'stride' words per ROM socket; ROMs smaller than the stride are mirrored (ROM_RELOAD).
static bool load_dcs(rom_pool &p, const rom_desc *r, int n, size_t stride, std::vector<u16> &dcs, std::string &err)
{
	dcs.assign(stride * n, 0xff);
	std::vector<u8> d;
	for (int k = 0; k < n; k++)
	{
		if (!load_desc(p, r[k], d, err)) return false;
		if (d.empty()) continue;
		for (size_t j = 0; j < stride; j++) dcs[k * stride + j] = d[j % d.size()];
	}
	return true;
}

static void mirror_to(std::vector<u8> &v, size_t n)
{
	if (v.empty()) { v.assign(n, 0); return; }
	size_t s = v.size();
	v.resize(n);
	for (size_t i = s; i < n; i++) v[i] = v[i % s];
}

std::unique_ptr<arcade_machine> game_catalog::create(const game_info &g, std::string &err)
{
	std::string id = g.id;
	rom_pool &p = pool();
	if (!from_roms(g, err)) return nullptr;
	std::vector<u8> lo, hi;
	if (id == "mk" || id == "mk2")
	{
		bool mk = id == "mk";
		const prog_version *v = mk ? find_version(p, mk_versions) : find_version(p, mk2_versions);
		tunit_roms r;
		if (!load_desc(p, v->lo, lo, err) || !load_desc(p, v->hi, hi, err)) return nullptr;
		interleave(lo, hi, r.program);
		if (mk)
		{
			if (!load_tunit_lanes(p, mk_gfx, 3, 0x80000, r.gfx, err)) return nullptr;
			if (!load_desc(p, mk_snd[0], r.snd_u3, err) || !load_desc(p, mk_snd[1], r.snd_u12, err) || !load_desc(p, mk_snd[2], r.snd_u13, err)) return nullptr;
			mirror_to(r.snd_u3, 0x40000);
			mirror_to(r.snd_u12, 0x80000);   // ROM_RELOAD of the 256K OKI ROMs
			mirror_to(r.snd_u13, 0x80000);
			return std::make_unique<tunit_machine>(std::move(r), tunit_game::mk);
		}
		if (!load_tunit_lanes(p, mk2_gfx, 3, 0x100000, r.gfx, err)) return nullptr;
		if (!load_dcs(p, mk2_dcs, 6, 0x100000, r.dcs, err)) return nullptr;
		return std::make_unique<tunit_machine>(std::move(r), tunit_game::mk2);
	}
	bool u = id == "umk3";
	const prog_version *v = u ? find_version(p, umk3_versions) : find_version(p, mk3_versions);
	wunit_roms r;
	if (!load_desc(p, v->lo, lo, err) || !load_desc(p, v->hi, hi, err)) return nullptr;
	interleave(lo, hi, r.program);
	if (!load_wolf_gfx(p, u ? umk3_gfx : mk3_gfx, 20, r.gfx, err)) return nullptr;
	if (!load_dcs(p, u ? umk3_dcs : mk3_dcs, 4, 0x100000, r.dcs, err)) return nullptr;
	return std::make_unique<wunit_machine>(std::move(r), u ? wunit_game::umk3 : wunit_game::mk3, 528);
}
