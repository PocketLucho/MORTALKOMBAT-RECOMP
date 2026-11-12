// tunit.cpp - Midway T-unit board (see header)
#include "tunit.h"

// Mortal Kombat T-unit protection sequence (MAME midtunit_m.cpp)
static const u8 mk_prot_values[] =
{
	0x13, 0x27, 0x0f, 0x1f, 0x3e, 0x3d, 0x3b, 0x37,
	0x2e, 0x1c, 0x38, 0x31, 0x22, 0x05, 0x0a, 0x15,
	0x2b, 0x16, 0x2d, 0x1a, 0x34, 0x28, 0x10, 0x21,
	0x03, 0x06, 0x0c, 0x19, 0x32, 0x24, 0x09, 0x13,
	0x27, 0x0f, 0x1f, 0x3e, 0x3d, 0x3b, 0x37, 0x2e,
	0x1c, 0x38, 0x31, 0x22, 0x05, 0x0a, 0x15, 0x2b,
	0x16, 0x2d, 0x1a, 0x34, 0x28, 0x10, 0x21, 0x03,
	0xff
};

static constexpr u32 CPU_CLOCK = 50000000;
static constexpr u32 PIXEL_CLOCK = 8000000 / 2;

tunit_machine::tunit_machine(tunit_roms roms, tunit_game game)
	: m_maincpu(machine_config(), "maincpu", nullptr, CPU_CLOCK)
	, m_video(m_machine, midtunit_video_device::kind::tunit)
	, m_roms(std::move(roms))
	, m_game(game)
{
	if (m_game == tunit_game::mk) m_sound = std::make_unique<williams_adpcm_sound>(m_machine);
	else m_dcs = std::make_unique<dcs_audio>(m_machine, dcs_audio::variant::k2);
	m_roms.program.resize(0x100000, 0xff);
	m_roms.gfx.resize(0xc00000, 0);
	m_ram.assign(0x40000, 0);
	m_nvram.assign(0x2000, 0);
	m_palram.assign(0x8000, 0);
	std::fill(std::begin(m_pens), std::end(m_pens), 0);

	m_mixer.init(m_machine, 48000);

	// screen: from TMS340 registers (MAME: set_raw(PIXEL_CLOCK*2, 506, 100, 500, 289, 20, 274))
	screen_device &scr = m_machine.screen();
	scr.set_raw(PIXEL_CLOCK * 2, 506, 100, 500, 289, 20, 274);
	scr.m_update_ind16 = [this](screen_device &s, bitmap_ind16 &bm, const rectangle &clip) { m_maincpu.tms340x0_ind16(s, bm, clip); };
	scr.on_vblank = [this] { capture_frame(); };

	// main CPU
	m_machine.add_cpu(m_maincpu);
	m_maincpu.set_bus(AS_PROGRAM, this);
	m_maincpu.set_halt_on_reset(false);
	m_maincpu.set_pixel_clock(PIXEL_CLOCK);
	m_maincpu.set_pixels_per_clock(2);
	m_maincpu.set_screen(scr);
	m_maincpu.set_scanline_ind16_callback([this](screen_device &s, bitmap_ind16 &b, int line, const tms340x0_device::display_params *p) { m_video.scanline_update(s, b, line, p); });
	m_maincpu.set_shiftreg_in_callback([this](offs_t a, u16 *sr) { m_video.to_shiftreg(a, sr); });
	m_maincpu.set_shiftreg_out_callback([this](offs_t a, u16 *sr) { m_video.from_shiftreg(a, sr); });
	m_maincpu.start();
	m_maincpu.m_pc_hint = [this] { return offs_t(m_maincpu.state_pc()); };

	if (m_game == tunit_game::mk2) m_video.set_gfx_rom_large(true);
	m_video.start(m_roms.gfx.data(), m_roms.gfx.size());
	m_video.dma_irq_cb = [this](int state) { m_maincpu.set_input_line(0, state); };

	if (m_sound)
	{
		m_sound->start(m_roms.snd_u3.data(), m_roms.snd_u3.size(), m_roms.snd_u12.data(), m_roms.snd_u13.data(), m_mixer);
		m_sound->install_hidden_ram(0xfb9c, 0xfbc6);    // sound chip protection (hidden RAM)
	}
	else
		m_dcs->start(m_roms.dcs, m_mixer);

	m_frame.assign(size_t(m_vis_w) * m_vis_h, 0);
	reset();
}

tunit_machine::~tunit_machine() = default;

void tunit_machine::reset()
{
	m_maincpu.reset();
	if (m_sound)
	{
		m_sound->reset();
		m_sound->reset_write(1);
		m_sound->reset_write(0);
	}
	else
	{
		m_dcs->reset_w(0);
		m_dcs->reset_w(1);
	}
	m_fake_sound_state = 0;
	m_mk_prot_index = 0;
	m_mk2_prot_data = 0;
}

double tunit_machine::refresh_hz() const
{
	return const_cast<running_machine &>(m_machine).screen().frame_period().as_hz();
}

void tunit_machine::run_frame()
{
	u64 start = m_frames;
	attotime limit = m_machine.m_basetime + attotime::from_msec(100);
	while (m_frames == start && m_machine.m_basetime < limit)
		m_machine.run_until(m_machine.m_basetime + attotime::from_usec(500));
}

void tunit_machine::capture_frame()
{
	screen_device &scr = m_machine.screen();
	const rectangle &vis = scr.visible_area();
	int w = vis.width(), h = vis.height();
	if (w != m_vis_w || h != m_vis_h)
	{
		m_vis_w = w; m_vis_h = h;
		m_frame.assign(size_t(w) * h, 0);
	}
	for (int y = 0; y < h; y++)
	{
		const u16 *src = &scr.m_bitmap.pix(vis.min_y + y, vis.min_x);
		u32 *dst = &m_frame[size_t(y) * w];
		for (int x = 0; x < w; x++) dst[x] = m_pens[src[x] & 0x7fff];
	}
	m_frames++;
}

void tunit_machine::set_inputs(const cabinet_inputs &in)
{
	// IN0: joystick, high punch, block, high kick (P1 low byte, P2 high byte)
	auto pl = [&](int p) -> u16 {
		const auto &q = in.p[p];
		u16 v = 0;
		if (q.up) v |= 0x01;
		if (q.down) v |= 0x02;
		if (q.left) v |= 0x04;
		if (q.right) v |= 0x08;
		if (q.hp) v |= 0x10;
		if (q.bl) v |= 0x20;
		if (q.hk) v |= 0x40;
		return v;
	};
	m_in0 = u16(~(pl(0) | (pl(1) << 8)));
	u16 v1 = 0, v2 = 0;
	if (in.p[0].coin) v1 |= 0x0001;
	if (in.p[1].coin) v1 |= 0x0002;
	if (in.p[0].start) v1 |= 0x0004;
	if (in.test) v1 |= 0x0010;
	if (in.p[1].start) v1 |= 0x0020;
	if (in.service) v1 |= 0x0040;
	if (m_game == tunit_game::mk)
	{
		// MK: low punch / low kick / second block live in IN1
		if (in.p[1].lp) v1 |= 0x0200;
		if (in.p[1].lk) v1 |= 0x0400;
		if (in.p[1].b6) v1 |= 0x0800;
		if (in.p[0].lp) v1 |= 0x1000;
		if (in.p[0].lk) v1 |= 0x2000;
		if (in.p[0].b6) v1 |= 0x8000;
	}
	else
	{
		if (in.volume_down) v1 |= 0x0800;
		if (in.volume_up) v1 |= 0x1000;
		// MK II: in IN2
		if (in.p[0].lp) v2 |= 0x0001;
		if (in.p[0].lk) v2 |= 0x0002;
		if (in.p[0].b6) v2 |= 0x0004;
		if (in.p[1].lp) v2 |= 0x0010;
		if (in.p[1].lk) v2 |= 0x0020;
		if (in.p[1].b6) v2 |= 0x0040;
	}
	m_in1 = u16(~v1);
	m_in2 = u16(~v2);
}

void tunit_machine::set_dips(const dip_settings &d)
{
	// MAME defaults: MK 0xfe7d (post test skipped), MK II 0xfc7d
	u16 v = m_game == tunit_game::mk ? 0xfe7d : 0xfc7d;
	if (d.test_switch) v &= ~0x0001;
	if (m_game == tunit_game::mk) { if (d.powerup_test) v |= 0x0100; else v &= ~0x0100; }
	else if (d.powerup_test) v |= 0x0200;
	// coinage: bits 2-3 country (USA 0x0c, French 0x08, German 0x04), bits 4-6 mode (0 = free play)
	v &= ~0x000c;
	v |= (d.country == 1) ? 0x0008 : (d.country == 2) ? 0x0004 : 0x000c;
	if (d.free_play) v = (v & ~0x0070) | 0x0080;   // free play from the dips, not the CMOS
	if (!d.attract_sound) v &= ~0x1000;
	if (!d.low_blows) v &= ~0x2000;
	if (!d.blood) v &= ~0x4000;
	if (!d.violence) v &= ~0x8000;
	m_dsw = v;
}

void tunit_machine::take_audio(std::vector<float> &out)
{
	m_mixer.render(out);
}

std::vector<u8> tunit_machine::nvram() const
{
	std::vector<u8> d(m_nvram.size() * 2);
	for (size_t i = 0; i < m_nvram.size(); i++) { d[i * 2] = u8(m_nvram[i]); d[i * 2 + 1] = u8(m_nvram[i] >> 8); }
	return d;
}

void tunit_machine::set_nvram(const std::vector<u8> &d)
{
	for (size_t i = 0; i < m_nvram.size() && i * 2 + 1 < d.size(); i++)
		m_nvram[i] = u16(d[i * 2] | (d[i * 2 + 1] << 8));
}

u32 tunit_machine::pc() const
{
	return m_maincpu.state_pc();
}

std::string tunit_machine::status() const
{
	if (m_sound)
		return string_format("frames=%llu dma=%llu snd: cmds=%u oki=%u ym=%u dac=%u", (unsigned long long)m_frames, (unsigned long long)m_video.m_dma_count, m_sound->m_cmds, m_sound->m_oki_cmds, m_sound->m_ym_writes, m_sound->m_dac_writes);
	return string_format("frames=%llu dma=%llu dcs in=%u out=%u", (unsigned long long)m_frames, (unsigned long long)m_video.m_dma_count, m_dcs->m_dbg_in, m_dcs->m_dbg_out);
}

// ---------------------------------------------------------------------------
// protection
// ---------------------------------------------------------------------------
u16 tunit_machine::mk_prot_r()
{
	if (m_mk_prot_index >= sizeof(mk_prot_values)) m_mk_prot_index = 0;
	return u16(mk_prot_values[m_mk_prot_index++] << 9);
}

void tunit_machine::mk_prot_w(u16 data, u16 mask)
{
	if (!(mask & 0xff00)) return;
	int const first_val = (data >> 9) & 0x3f;
	m_mk_prot_index = 0;
	for (size_t i = 0; i < sizeof(mk_prot_values); i++)
		if (mk_prot_values[i] == first_val) { m_mk_prot_index = i; break; }
}

// ---------------------------------------------------------------------------
// memory map (addresses are TMS34010 bit addresses)
// ---------------------------------------------------------------------------
u64 tunit_machine::read(offs_t addr, u64 mem_mask) { return read16(addr, u16(mem_mask)); }
void tunit_machine::write(offs_t addr, u64 data, u64 mem_mask) { write16(addr, u16(data), u16(mem_mask)); }

#define IN_RANGE(a, s, e) ((a) >= (s) && (a) <= (e))
#define WOFF(a, s) (((a) - (s)) >> 4)

u16 tunit_machine::read16(offs_t a, u16 mask)
{
	if (a >= 0xff800000) return u16(m_roms.program[WOFF(a, 0xff800000) * 2] | (m_roms.program[WOFF(a, 0xff800000) * 2 + 1] << 8));
	if (IN_RANGE(a, 0x01000000, 0x013fffff)) return m_ram[WOFF(a, 0x01000000)];
	if (a < 0x00400000) return m_video.midtunit_vram_r(WOFF(a, 0));
	if (IN_RANGE(a, 0x02000000, 0x07ffffff)) return m_video.midtunit_gfxrom_r(WOFF(a, 0x02000000));
	if (m_game == tunit_game::mk)
	{
		if (IN_RANGE(a, 0x01b00000, 0x01b6ffff)) return mk_prot_r();
	}
	else
	{
		if (IN_RANGE(a, 0x01a190e0, 0x01a190ff) || IN_RANGE(a, 0x01a3d0c0, 0x01a3d0ff)) return m_mk2_prot_data;
		if (IN_RANGE(a, 0x01a191c0, 0x01a191df)) return u16(m_mk2_prot_data >> 1);
		if (IN_RANGE(a, 0x01d9d1e0, 0x01d9d1ff) || IN_RANGE(a, 0x01def920, 0x01def93f)) return 2;
	}
	if (IN_RANGE(a, 0x01400000, 0x0141ffff)) return m_nvram[WOFF(a, 0x01400000) & 0x1fff];
	if (IN_RANGE(a, 0x01600000, 0x0160003f))
	{
		switch (WOFF(a, 0x01600000))
		{
			case 0: return m_in0;
			case 1: return m_in1;
			case 2: return m_in2;
			default: return m_dsw;
		}
	}
	if (IN_RANGE(a, 0x01800000, 0x0187ffff)) return m_palram[WOFF(a, 0x01800000)];
	if (IN_RANGE(a, 0x01a80000, 0x01a800ff)) return m_video.dma_r(WOFF(a, 0x01a80000));
	if (IN_RANGE(a, 0x01d00000, 0x01d0001f))
	{
		if (m_dcs) return u16(m_dcs->control_r() >> 4);
		if (m_fake_sound_state) { m_fake_sound_state--; return 0; }
		return 0xffff;
	}
	if (IN_RANGE(a, 0x01d01020, 0x01d0103f)) return m_dcs ? u16(m_dcs->data_r() & 0xff) : 0xffff;
	if (IN_RANGE(a, 0xc0000000, 0xc00001ff)) return m_maincpu.io_register_r(WOFF(a, 0xc0000000));
	if (IN_RANGE(a, 0x1f800000, 0x1fffffff)) return u16(m_roms.program[WOFF(a, 0x1f800000) * 2] | (m_roms.program[WOFF(a, 0x1f800000) * 2 + 1] << 8));
	return 0xffff;
}

void tunit_machine::write16(offs_t a, u16 data, u16 mask)
{
	if (IN_RANGE(a, 0x01000000, 0x013fffff)) { u16 &r = m_ram[WOFF(a, 0x01000000)]; r = u16((r & ~mask) | (data & mask)); return; }
	if (a < 0x00400000) { m_video.midtunit_vram_w(WOFF(a, 0), data, mask); return; }
	if (m_game == tunit_game::mk)
	{
		if (IN_RANGE(a, 0x01b00000, 0x01b6ffff)) { mk_prot_w(data, mask); return; }
	}
	else
	{
		if (IN_RANGE(a, 0x00f20c60, 0x00f20c7f) || IN_RANGE(a, 0x00f42820, 0x00f4283f)) { m_mk2_prot_data = u16((m_mk2_prot_data & ~mask) | (data & mask)); return; }
	}
	if (IN_RANGE(a, 0x01400000, 0x0141ffff)) { u16 &r = m_nvram[WOFF(a, 0x01400000) & 0x1fff]; r = u16((r & ~mask) | (data & mask)); return; }
	if (IN_RANGE(a, 0x01480000, 0x014fffff)) return; // cmos enable
	if (IN_RANGE(a, 0x01800000, 0x0187ffff))
	{
		offs_t o = WOFF(a, 0x01800000);
		u16 &r = m_palram[o];
		r = u16((r & ~mask) | (data & mask));
		// xRGB_555
		u32 rr = (r >> 10) & 0x1f, gg = (r >> 5) & 0x1f, bb = r & 0x1f;
		rr = (rr << 3) | (rr >> 2); gg = (gg << 3) | (gg >> 2); bb = (bb << 3) | (bb >> 2);
		m_pens[o] = 0xff000000 | (rr << 16) | (gg << 8) | bb;
		return;
	}
	if (IN_RANGE(a, 0x01a80000, 0x01a800ff)) { m_video.dma_w(WOFF(a, 0x01a80000), data, mask); return; }
	if (IN_RANGE(a, 0x01b00000, 0x01b0001f) || IN_RANGE(a, 0x01f00000, 0x01f0001f)) { m_video.midtunit_control_w(0, data, mask); return; }
	if (IN_RANGE(a, 0x01d01020, 0x01d0103f))
	{
		offs_t offset = WOFF(a, 0x01d01020);
		if (!offset) return;
		if ((mask & 0x00ff) && (mask & 0xff00))
		{
			if (m_dcs)
			{
				m_dcs->reset_w(data & 0x100);
				m_dcs->data_w(data & 0xff);
			}
			else
			{
				m_sound->reset_write(~data & 0x100);
				m_sound->write(data & 0xff);
				m_fake_sound_state = 128;   // the games check for $82 loops
			}
		}
		return;
	}
	if (IN_RANGE(a, 0x01d81060, 0x01d8107f)) return; // watchdog
	if (IN_RANGE(a, 0xc0000000, 0xc00001ff)) { m_maincpu.io_register_w(WOFF(a, 0xc0000000), data, mask); return; }
}
