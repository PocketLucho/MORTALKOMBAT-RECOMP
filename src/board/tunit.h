// tunit.h - Midway T-unit (Mortal Kombat rev 5.0 T-Unit with the Williams ADPCM board,
// Mortal Kombat II with the DCS board). Memory map and protection follow MAME
// midtunit.cpp / midtunit_m.cpp (BSD-3-Clause).
#pragma once
#include "machine.h"
#include "midway_video.h"
#include "williams_adpcm.h"
#include "dcs.h"
#include <memory>
#include "cpu/tms34010/tms34010.h"

struct tunit_roms
{
	std::vector<u8> program;  // 0x100000 bytes, little-endian words (uj12 low / ug12 high interleaved)
	std::vector<u8> gfx;      // 0xc00000 bytes
	// MK: Williams ADPCM board
	std::vector<u8> snd_u3;   // 0x40000 (6809 program)
	std::vector<u8> snd_u12;  // 0x80000 (OKI, a 256K ROM is mirrored)
	std::vector<u8> snd_u13;  // 0x80000
	// MK II: DCS board, one ROM byte per word
	std::vector<u16> dcs;
};

// exposes a few internals of MAME's core for diagnostics
class tms34010_cpu : public tms34010_device
{
public:
	using tms34010_device::tms34010_device;
	u32 state_pc() const { return m_pc; }
	u32 reg(int n) const { return u32(m_regs[n].reg); }
};

enum class tunit_game { mk, mk2 };

class tunit_machine : public arcade_machine, private mem_bus
{
public:
	tunit_machine(tunit_roms roms, tunit_game game);
	~tunit_machine() override;

	void reset() override;
	void run_frame() override;
	int width() const override { return m_vis_w; }
	int height() const override { return m_vis_h; }
	const u32 *frame() const override { return m_frame.data(); }
	double refresh_hz() const override;
	void set_inputs(const cabinet_inputs &in) override;
	void set_dips(const dip_settings &d) override;
	void take_audio(std::vector<float> &out) override;
	std::vector<u8> nvram() const override;
	void set_nvram(const std::vector<u8> &d) override;
	std::string status() const override;
	u32 pc() const override;
	u32 state_hash() const override { return fnv1a(m_ram.data(), m_ram.size() * 2); }
	u32 fingerprint() const override
	{
		u32 h = fnv1a(m_roms.program.data(), m_roms.program.size());
		h = fnv1a(m_roms.snd_u3.data(), m_roms.snd_u3.size(), h);
		h = fnv1a(m_roms.snd_u12.data(), m_roms.snd_u12.size(), h);
		h = fnv1a(m_roms.snd_u13.data(), m_roms.snd_u13.size(), h);
		return fnv1a(m_roms.dcs.data(), m_roms.dcs.size() * 2, h);
	}
	cpu_device *maincpu() override { return &m_maincpu; }

private:
	// mem_bus for the TMS34010 (bit addresses)
	u64 read(offs_t addr, u64 mem_mask) override;
	void write(offs_t addr, u64 data, u64 mem_mask) override;
	u16 read16(offs_t a, u16 mask);
	void write16(offs_t a, u16 data, u16 mask);

	u16 mk_prot_r();
	void mk_prot_w(u16 data, u16 mask);
	void capture_frame();

	running_machine m_machine;
	tms34010_cpu m_maincpu;
	midtunit_video_device m_video;
	std::unique_ptr<williams_adpcm_sound> m_sound;   // MK
	std::unique_ptr<dcs_audio> m_dcs;                // MK II
	audio_mixer m_mixer;
	tunit_roms m_roms;
	tunit_game m_game;

	std::vector<u16> m_ram;        // 0x40000 words
	std::vector<u16> m_nvram;      // 0x2000 words
	std::vector<u16> m_palram;     // 0x8000 words
	u32 m_pens[0x8000];
	u16 m_in0 = 0xffff, m_in1 = 0xffff, m_in2 = 0xffff, m_dsw = 0x7ffd;
	int m_fake_sound_state = 0;
	size_t m_mk_prot_index = 0;
	u16 m_mk2_prot_data = 0;

	std::vector<u32> m_frame;
	int m_vis_w = 400, m_vis_h = 254;
	u64 m_frames = 0;
};
