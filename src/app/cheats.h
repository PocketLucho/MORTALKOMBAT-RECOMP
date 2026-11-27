// cheats.h - per-revision cheats: RAM values held every frame (infinite energy / run),
// one-shot RAM writes (pick the next opponent) and CMOS unlocks (hidden characters),
// which are written with the settings checksum fixed and take effect after a reset.
// Addresses come from the MAME 2003 cheat database (byte address of the 34010 word, value
// in its low byte) and were checked against the games.
#pragma once
#include "machine.h"
#include <string>
#include <vector>

enum class cheat_kind { hold, once, cmos };

struct cheat_poke { u32 addr; u8 on; u8 off; };   // MAME-2003 byte address; low byte of the word

struct cheat_def
{
	const char *game;       // catalogue id
	const char *version;    // program revision label ("5.0", "L3.1", "1.0", "1.2"...); "*" = any
	const char *id;         // stable key for the settings file
	const char *es, *en;    // name
	const char *help_es, *help_en;
	cheat_kind kind;
	std::vector<cheat_poke> pokes;
	u32 ck_hi = 0, ck_lo = 0;   // CMOS checksum bytes (~sum of the settings), 0 = none
};

// cheats available for a game revision
std::vector<const cheat_def *> cheats_for(const std::string &game, const std::string &version);

// hold cheats: call once per frame for the enabled ones
void cheat_hold(arcade_machine &m, const cheat_def &c);
// one-shot cheats: writes now
void cheat_once(arcade_machine &m, const cheat_def &c);
// CMOS cheats: on/off (writes the values and fixes the checksum); the caller resets the board
bool cheat_cmos_set(arcade_machine &m, const cheat_def &c, bool on);
bool cheat_cmos_get(arcade_machine &m, const cheat_def &c);
