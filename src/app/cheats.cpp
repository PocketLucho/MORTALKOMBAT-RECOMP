// cheats.cpp - see cheats.h
#include "cheats.h"

// address -> 34010 bit address of the word that holds it
static u32 bitaddr(u32 a) { return a * 8; }

static const std::vector<cheat_def> &all_cheats()
{
	static const std::vector<cheat_def> list = {
		// ---- Mortal Kombat rev 5.0 / 4.0 (T-Unit)
		{ "mk", "*", "energy1", "Energia infinita J1", "Infinite energy P1", "La barra del jugador 1 no baja.", "Player 1's bar never goes down.",
		  cheat_kind::hold, { { 0x20A260, 0xA1, 0 } } },
		{ "mk", "*", "energy2", "Energia infinita J2", "Infinite energy P2", "La barra del jugador 2 (o de la CPU) no baja.", "Player 2's (or the CPU's) bar never goes down.",
		  cheat_kind::hold, { { 0x20A2BC, 0xA1, 0 } } },
		{ "mk", "*", "pit", "Ir al fondo del Foso", "Go to the bottom of the Pit",
		  "Usalo en la pantalla de seleccion: el siguiente combate es en el fondo del Foso (The Pit).",
		  "Use it on the select screen: the next fight is at the bottom of the Pit.", cheat_kind::once, { { 0x20A36E, 0x09, 0 } } },
		// ---- Mortal Kombat II rev L3.1
		{ "mk2", "L3.1", "energy1", "Energia infinita J1", "Infinite energy P1", "La barra del jugador 1 no baja.", "Player 1's bar never goes down.",
		  cheat_kind::hold, { { 0x20BCA0, 0xA1, 0 }, { 0x20C05E, 0xA1, 0 } } },
		{ "mk2", "L3.1", "energy2", "Energia infinita J2", "Infinite energy P2", "La barra del jugador 2 (o de la CPU) no baja.", "Player 2's (or the CPU's) bar never goes down.",
		  cheat_kind::hold, { { 0x20BC88, 0xA1, 0 }, { 0x20C1D8, 0xA1, 0 } } },
		{ "mk2", "L3.1", "smoke", "Siguiente rival: Smoke", "Next opponent: Smoke",
		  "Partida de 1 jugador: usalo en la seleccion de personaje para luchar contra Smoke.",
		  "1-player game: use it on the character select screen to fight Smoke.", cheat_kind::once,
		  { { 0x20C358, 0x09, 0 }, { 0x20C35A, 0x08, 0 }, { 0x20C35C, 0x0B, 0 }, { 0x20C366, 0x0B, 0 } } },
		{ "mk2", "L3.1", "jade", "Siguiente rival: combate antes de \"?\" (Jade)", "Next opponent: the match before \"?\" (Jade)",
		  "Partida de 1 jugador: gana un asalto usando solo patada baja y bloqueo para luchar contra Jade.",
		  "1-player game: win a round using only low kick and block to fight Jade.", cheat_kind::once,
		  { { 0x20C358, 0x01, 0 }, { 0x20C35A, 0x09, 0 }, { 0x20C35C, 0x0A, 0 }, { 0x20C366, 0x0A, 0 } } },
		{ "mk2", "L3.1", "noob", "El ganador lucha contra Noob Saibot", "Winner fights Noob Saibot",
		  "Partida de 2 jugadores: activalo despues de empezar; el ganador del combate se enfrenta a Noob Saibot.",
		  "2-player game: turn it on once started; the winner of the fight faces Noob Saibot.", cheat_kind::once, { { 0x20C36A, 0x32, 0 } } },
		// ---- Mortal Kombat 3 rev 1.0
		{ "mk3", "1.0", "energy1", "Energia infinita J1", "Infinite energy P1", "La barra del jugador 1 no baja.", "Player 1's bar never goes down.",
		  cheat_kind::hold, { { 0x20C058, 0xA6, 0 } } },
		{ "mk3", "1.0", "energy2", "Energia infinita J2", "Infinite energy P2", "La barra del jugador 2 (o de la CPU) no baja.", "Player 2's (or the CPU's) bar never goes down.",
		  cheat_kind::hold, { { 0x20C1CE, 0xA6, 0 } } },
		{ "mk3", "1.0", "run1", "Correr infinito J1", "Infinite run P1", "La barra de correr del jugador 1 no se gasta.", "Player 1's run bar never runs out.",
		  cheat_kind::hold, { { 0x20C05A, 0x30, 0 } } },
		{ "mk3", "1.0", "run2", "Correr infinito J2", "Infinite run P2", "La barra de correr del jugador 2 no se gasta.", "Player 2's run bar never runs out.",
		  cheat_kind::hold, { { 0x20C1D0, 0x30, 0 } } },
		// ---- Mortal Kombat 3 rev 2.0 / 2.1
		{ "mk3", "2.1", "energy1", "Energia infinita J1", "Infinite energy P1", "La barra del jugador 1 no baja.", "Player 1's bar never goes down.",
		  cheat_kind::hold, { { 0x20C148, 0xA6, 0 } } },
		{ "mk3", "2.1", "energy2", "Energia infinita J2", "Infinite energy P2", "La barra del jugador 2 (o de la CPU) no baja.", "Player 2's (or the CPU's) bar never goes down.",
		  cheat_kind::hold, { { 0x20C2BE, 0xA6, 0 } } },
		{ "mk3", "2.1", "run1", "Correr infinito J1", "Infinite run P1", "La barra de correr del jugador 1 no se gasta.", "Player 1's run bar never runs out.",
		  cheat_kind::hold, { { 0x20C14A, 0x30, 0 } } },
		{ "mk3", "2.1", "run2", "Correr infinito J2", "Infinite run P2", "La barra de correr del jugador 2 no se gasta.", "Player 2's run bar never runs out.",
		  cheat_kind::hold, { { 0x20C2C0, 0x30, 0 } } },
		// ---- Ultimate Mortal Kombat 3 rev 1.2 / 1.1
		{ "umk3", "1.2", "energy1", "Energia infinita J1", "Infinite energy P1", "La barra del jugador 1 no baja.", "Player 1's bar never goes down.",
		  cheat_kind::hold, { { 0x20C14C, 0xA6, 0 } } },
		{ "umk3", "1.2", "energy2", "Energia infinita J2", "Infinite energy P2", "La barra del jugador 2 (o de la CPU) no baja.", "Player 2's (or the CPU's) bar never goes down.",
		  cheat_kind::hold, { { 0x20C2C2, 0xA6, 0 } } },
		{ "umk3", "1.2", "run1", "Correr infinito J1", "Infinite run P1", "La barra de correr del jugador 1 no se gasta.", "Player 1's run bar never runs out.",
		  cheat_kind::hold, { { 0x20C14E, 0x30, 0 } } },
		{ "umk3", "1.2", "run2", "Correr infinito J2", "Infinite run P2", "La barra de correr del jugador 2 no se gasta.", "Player 2's run bar never runs out.",
		  cheat_kind::hold, { { 0x20C2C4, 0x30, 0 } } },
	};
	return list;
}

// CMOS unlocks: the settings area is protected by a checksum (~sum of the bytes, two bytes
// hi / lo); the hidden-character flags are 0xB0ED stored in two consecutive bytes
static const std::vector<cheat_def> &cmos_cheats()
{
	static const std::vector<cheat_def> list = {
		{ "mk3", "*", "smoke", "Desbloquear Smoke", "Unlock Smoke",
		  "Smoke queda seleccionable (se guarda en la CMOS). Reinicia la placa al cambiarlo.",
		  "Smoke becomes selectable (stored in the CMOS). Resets the board when changed.", cheat_kind::cmos,
		  { { 0x281084, 0xED, 0x00 }, { 0x281086, 0xB0, 0x00 } }, 0x281160, 0x281162 },
		{ "umk3", "*", "mileena", "Desbloquear Mileena", "Unlock Mileena",
		  "Mileena aparece en la seleccion de personaje (se guarda en la CMOS). Reinicia la placa al cambiarlo.",
		  "Mileena appears on the select screen (stored in the CMOS). Resets the board when changed.", cheat_kind::cmos,
		  { { 0x2810FC, 0xED, 0x00 }, { 0x2810FE, 0xB0, 0x00 } }, 0x281160, 0x281162 },
		{ "umk3", "*", "ermac", "Desbloquear Ermac", "Unlock Ermac",
		  "Ermac aparece en la seleccion de personaje (se guarda en la CMOS). Reinicia la placa al cambiarlo.",
		  "Ermac appears on the select screen (stored in the CMOS). Resets the board when changed.", cheat_kind::cmos,
		  { { 0x281104, 0xED, 0x00 }, { 0x281106, 0xB0, 0x00 } }, 0x281160, 0x281162 },
		{ "umk3", "*", "classicsz", "Desbloquear Sub-Zero clasico", "Unlock Classic Sub-Zero",
		  "El Sub-Zero clasico aparece en la seleccion de personaje (se guarda en la CMOS). Reinicia la placa al cambiarlo.",
		  "Classic Sub-Zero appears on the select screen (stored in the CMOS). Resets the board when changed.", cheat_kind::cmos,
		  { { 0x28110C, 0xED, 0x00 }, { 0x28110E, 0xB0, 0x00 } }, 0x281160, 0x281162 },
	};
	return list;
}

std::vector<const cheat_def *> cheats_for(const std::string &game, const std::string &version)
{
	std::vector<const cheat_def *> r;
	std::string v = version;
	if (game == "mk3" && v == "2.0") v = "2.1";
	if (game == "umk3" && v == "1.1") v = "1.2";
	for (auto *l : { &cmos_cheats(), &all_cheats() })
		for (auto &c : *l)
			if (game == c.game && (std::string(c.version) == "*" || v == c.version)) r.push_back(&c);
	return r;
}

void cheat_hold(arcade_machine &m, const cheat_def &c)
{
	for (auto &p : c.pokes) m.cheat_write(bitaddr(p.addr), p.on, 0x00ff);
}

void cheat_once(arcade_machine &m, const cheat_def &c)
{
	cheat_hold(m, c);
}

static int low_byte(arcade_machine &m, u32 a)
{
	u16 v = 0;
	return m.cheat_read(bitaddr(a), v) ? (v & 0xff) : -1;
}

bool cheat_cmos_get(arcade_machine &m, const cheat_def &c)
{
	for (auto &p : c.pokes)
		if (low_byte(m, p.addr) != p.on) return false;
	return true;
}

bool cheat_cmos_set(arcade_machine &m, const cheat_def &c, bool on)
{
	int delta = 0;
	for (auto &p : c.pokes)
	{
		int old = low_byte(m, p.addr);
		if (old < 0) return false;
		int nv = on ? p.on : p.off;
		delta += nv - old;
		m.cheat_write(bitaddr(p.addr), u16(nv), 0x00ff);
	}
	if (c.ck_hi)
	{
		// checksum = ~sum: it goes down by what the bytes went up
		int hi = low_byte(m, c.ck_hi), lo = low_byte(m, c.ck_lo);
		if (hi < 0 || lo < 0) return false;
		u16 ck = u16(((hi << 8) | lo) - delta);
		m.cheat_write(bitaddr(c.ck_hi), ck >> 8, 0x00ff);
		m.cheat_write(bitaddr(c.ck_lo), ck & 0xff, 0x00ff);
	}
	return true;
}
