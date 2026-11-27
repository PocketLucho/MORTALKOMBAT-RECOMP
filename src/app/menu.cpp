// menu.cpp - the menus: launcher (game list + status card), settings / pause pages with tabs
// (game, cheats, video, controls, machine, system), online game, about; drawn on a
// window-resolution canvas (ui_draw.h) over the game picture.
#include "app.h"
#include "lang.h"
#include "shaders.h"

#include <algorithm>
#include <cmath>
#include <cstring>

// ---------------------------------------------------------------------------
// look: near-black crimson, blood red accents, bone text
// ---------------------------------------------------------------------------
static const ucol kBlood = { 214, 20, 24 }, kDeep = { 120, 0, 8 }, kBone = { 238, 226, 206 }, kDim = { 172, 142, 132 },
                  kPanelTop = { 38, 7, 9 }, kPanelBot = { 11, 2, 3 }, kFrame = { 66, 13, 15 }, kGold = { 255, 196, 92 },
                  kBlack = { 0, 0, 0 }, kWhite = { 255, 250, 244 };

enum { P_GAME, P_CHEATS, P_VIDEO, P_CONTROLS, P_MACHINE, P_SYSTEM };
enum { K_ACTION, K_CHOICE, K_TOGGLE, K_SLIDER, K_BIND, K_INFO };
enum
{
	R_RESUME, R_RESET, R_SHOT, R_TEST, R_LAUNCHER, R_QUIT,
	R_CHEAT,
	R_SHADER, R_SCAN, R_ASPECT, R_INTEGER, R_FULL, R_VOLUME,
	R_PLAYER, R_BIND, R_HK_MENU, R_HK_SHADER, R_RESET_KEYS,
	R_FREE, R_BLOOD, R_VIOLENCE, R_LOWBLOWS, R_ATTRACT, R_COUNTRY,
	R_LANG, R_ROMDIR, R_RESCAN, R_RECOMP, R_ABOUT,
	R_NONE
};

struct app::row
{
	int id = R_NONE, kind = K_ACTION;
	std::string label, value, help;
	bool on = false;
	float frac = 0;
	int act = -1;                        // R_BIND: action; R_CHEAT: cheat index
	std::string k1, k2;                  // R_BIND: keyboard key / pad input
};

static const char *aspect_name(int a)
{
	static const char *es[4] = { "4:3 original", "16:9 panoramico", "16:10 panoramico", "Llenar la ventana" };
	static const char *en[4] = { "4:3 original", "16:9 widescreen", "16:10 widescreen", "Fill the window" };
	return (g_lang_en ? en : es)[std::clamp(a, 0, 3)];
}
static const char *hk_menu_name(int v)
{
	static const char *n[HKM_COUNT] = { "Back + Start", "Guide", "L3 + R3" };
	return n[std::clamp(v, 0, HKM_COUNT - 1)];
}
static const char *hk_shader_name(int v)
{
	static const char *es[HKS_COUNT] = { "Ninguno", "R3 (clic stick dcho.)", "L3 (clic stick izq.)", "Back + RB", "Back + LB" };
	static const char *en[HKS_COUNT] = { "None", "R3 (right stick click)", "L3 (left stick click)", "Back + RB", "Back + LB" };
	return (g_lang_en ? en : es)[std::clamp(v, 0, HKS_COUNT - 1)];
}
static const char *country_name(int c)
{
	static const char *es[3] = { "EE. UU.", "Francia", "Alemania" }, *en[3] = { "USA", "France", "Germany" };
	return (g_lang_en ? en : es)[std::clamp(c, 0, 2)];
}

// ---------------------------------------------------------------------------
// cheats
// ---------------------------------------------------------------------------
std::vector<const cheat_def *> app::current_cheats()
{
	if (!m_mach || m_cur_game < 0) return {};
	return cheats_for(game_list()[size_t(m_cur_game)].id, m_version);
}

bool app::cheat_on(const cheat_def &c) const
{
	if (c.kind == cheat_kind::cmos) return m_mach && cheat_cmos_get(*m_mach, c);
	if (c.kind == cheat_kind::once || m_cur_game < 0) return false;
	auto it = m_cfg.cheats.find(game_list()[size_t(m_cur_game)].id);
	return it != m_cfg.cheats.end() && ("," + it->second + ",").find("," + std::string(c.id) + ",") != std::string::npos;
}

void app::set_cheat(const cheat_def &c, bool on)
{
	if (!m_mach) return;
	if (c.kind == cheat_kind::once) { cheat_once(*m_mach, c); message(std::string(tr("Truco aplicado: ", "Cheat applied: ")) + (g_lang_en ? c.en : c.es), 2500); return; }
	if (c.kind == cheat_kind::cmos)
	{
		if (cheat_cmos_set(*m_mach, c, on))
		{
			m_mach->reset();
			message(tr("Guardado en la CMOS; placa reiniciada", "Stored in the CMOS; board reset"), 3000);
		}
		return;
	}
	std::string g = game_list()[size_t(m_cur_game)].id, &ids = m_cfg.cheats[g];
	std::vector<std::string> list;
	size_t p = 0;
	while (p < ids.size()) { size_t e = ids.find(',', p); if (e == std::string::npos) e = ids.size(); if (e > p && ids.substr(p, e - p) != c.id) list.push_back(ids.substr(p, e - p)); p = e + 1; }
	if (on) list.push_back(c.id);
	ids.clear();
	for (auto &s : list) ids += (ids.empty() ? "" : ",") + s;
}

// ---------------------------------------------------------------------------
// pages
// ---------------------------------------------------------------------------
std::vector<int> app::page_list() const
{
	if (m_menu_open) return { P_GAME, P_CHEATS, P_VIDEO, P_CONTROLS, P_MACHINE, P_SYSTEM };
	return { P_VIDEO, P_CONTROLS, P_MACHINE, P_SYSTEM };
}

static const char *page_name(int p)
{
	switch (p)
	{
		case P_GAME: return tr("PARTIDA", "GAME");
		case P_CHEATS: return tr("TRUCOS", "CHEATS");
		case P_VIDEO: return tr("VIDEO", "VIDEO");
		case P_CONTROLS: return tr("CONTROLES", "CONTROLS");
		case P_MACHINE: return tr("MAQUINA", "MACHINE");
		default: return tr("SISTEMA", "SYSTEM");
	}
}

std::vector<app::row> app::page_rows(int page)
{
	std::vector<row> r;
	auto add = [&](int id, int kind, const std::string &label, const std::string &help, const std::string &value = "", bool on = false) {
		row x;
		x.id = id; x.kind = kind; x.label = label; x.help = help; x.value = value; x.on = on;
		r.push_back(x);
		return &r.back();
	};
	switch (page)
	{
		case P_GAME:
			add(R_RESUME, K_ACTION, tr("Continuar", "Resume"), tr("Vuelve a la partida.", "Back to the game."));
			if (!m_net_game)
			{
				add(R_RESET, K_ACTION, tr("Reiniciar la placa", "Reset the board"), tr("Como el boton de reset del mueble (F3).", "Like the cabinet's reset button (F3)."));
				add(R_TEST, K_TOGGLE, tr("Interruptor TEST", "TEST switch"), tr("Menu de servicio de la recreativa: ajustes, auditorias, pruebas (F2).",
				                                                                 "The arcade's service menu: adjustments, audits, tests (F2)."), "", m_test);
			}
			add(R_SHOT, K_ACTION, tr("Captura de pantalla", "Screenshot"), tr("Guarda la imagen en la carpeta 'screenshots' (F12).", "Saves the picture in the 'screenshots' folder (F12)."));
			add(R_LAUNCHER, K_ACTION, m_net_game ? tr("Salir de la partida en red", "Leave the online game") : tr("Volver al lanzador", "Back to the launcher"),
			    tr("Guarda la CMOS (puntuaciones, ajustes) y vuelve a la lista de juegos.", "Saves the CMOS (scores, settings) and goes back to the game list."));
			add(R_QUIT, K_ACTION, tr("Salir al escritorio", "Quit to desktop"), tr("Guarda todo y cierra el programa.", "Saves everything and closes the program."));
			break;
		case P_CHEATS:
		{
			auto list = current_cheats();
			if (m_net_game) add(R_NONE, K_INFO, tr("No disponibles en partidas en red", "Not available in online games"), "");
			else if (list.empty()) add(R_NONE, K_INFO, tr("No hay trucos para esta revision", "No cheats for this revision"), "");
			else
				for (size_t i = 0; i < list.size(); i++)
				{
					const cheat_def &c = *list[i];
					row *x = add(R_CHEAT, c.kind == cheat_kind::once ? K_ACTION : K_TOGGLE, g_lang_en ? c.en : c.es, g_lang_en ? c.help_en : c.help_es, "", cheat_on(c));
					x->act = int(i);
				}
			break;
		}
		case P_VIDEO:
			add(R_SHADER, K_CHOICE, tr("Filtro de imagen", "Picture filter"),
			    tr("Nitido, suavizado, scanlines o CRT (tambien F4 o el atajo del mando).", "Sharp, smooth, scanlines or CRT (also F4 or the pad shortcut)."), shader_name(m_cfg.shader));
			r.push_back(row{ R_SCAN, K_SLIDER, tr("Intensidad de las scanlines", "Scanline strength"), std::to_string(int(m_cfg.scan_strength * 100 + 0.5f)) + "%",
			                 tr("Solo con el filtro Scanlines.", "Scanlines filter only."), false, m_cfg.scan_strength });
			add(R_ASPECT, K_CHOICE, tr("Forma de la imagen", "Picture shape"), tr("4:3 como el monitor original, o estirada a panoramico (F7).", "4:3 like the original monitor, or stretched to widescreen (F7)."), aspect_name(m_cfg.aspect));
			add(R_INTEGER, K_TOGGLE, tr("Escalado entero", "Integer scaling"), tr("Pixeles todos iguales (bandas a los lados); si no, llena la pantalla (F5).", "Every pixel the same size (bars at the sides); otherwise fills the screen (F5)."), "", m_cfg.integer_scale);
			add(R_FULL, K_TOGGLE, tr("Pantalla completa", "Fullscreen"), tr("Tambien con F11 o Alt+Intro.", "Also with F11 or Alt+Enter."), "", m_cfg.fullscreen);
			r.push_back(row{ R_VOLUME, K_SLIDER, tr("Volumen", "Volume"), std::to_string(int(m_cfg.volume * 100 + 0.5f)) + "%", tr("Tambien con - y +.", "Also with - and +."), false, m_cfg.volume });
			break;
		case P_CONTROLS:
		{
			add(R_PLAYER, K_CHOICE, tr("Jugador", "Player"), tr("Teclas de cada jugador. El mando es comun: el primer mando es el J1, el segundo el J2.",
			                                                     "Keys of each player. The pad layout is shared: the first pad is P1, the second P2."),
			    std::string(tr("Jugador ", "Player ")) + std::to_string(m_ctrl_player + 1));
			for (int a = 0; a < ACT_COUNT; a++)
			{
				row x;
				x.id = R_BIND; x.kind = K_BIND; x.act = a; x.label = action_name(a);
				SDL_Scancode sc = m_cfg.keys[m_ctrl_player][a];
				x.k1 = sc == SDL_SCANCODE_UNKNOWN ? "-" : SDL_GetScancodeName(sc);
				x.k2 = pad_input_name(m_cfg.pad[a][0]);
				if (m_cfg.pad[a][1] != PAD_NONE) x.k2 += std::string(" / ") + pad_input_name(m_cfg.pad[a][1]);
				x.help = std::string(tr("Izquierda: tecla del jugador ", "Left: key of player ")) + std::to_string(m_ctrl_player + 1) +
				         tr("; derecha: boton de todos los mandos. A / Intro y pulsa la tecla o el boton nuevo.",
				            "; right: button of every pad. A / Enter, then press the new key or button.");
				r.push_back(x);
			}
			add(R_HK_MENU, K_CHOICE, tr("Atajo del mando: menu", "Pad shortcut: menu"), tr("Abre este menu durante la partida (Steam usa el boton Guide).",
			    "Opens this menu while playing (Steam takes the Guide button)."), hk_menu_name(m_cfg.hk_menu));
			add(R_HK_SHADER, K_CHOICE, tr("Atajo del mando: filtro", "Pad shortcut: filter"), tr("Cambia el filtro de imagen durante la partida. En Steam Deck puedes asignar L4/R4 a L3/R3 en Steam Input.",
			    "Changes the picture filter while playing. On the Steam Deck you can map L4/R4 to L3/R3 in Steam Input."), hk_shader_name(m_cfg.hk_shader));
			add(R_RESET_KEYS, K_ACTION, tr("Restablecer los controles", "Reset the controls"), tr("Teclas y mando como al principio.", "Keys and pad as they came."));
			break;
		}
		case P_MACHINE:
		{
			std::string h = m_menu_open ? tr("Se aplica al instante; algunos juegos lo leen al reiniciar.", "Applies at once; some games read it at reset.")
			                            : tr("Interruptores DIP de la placa; se aplican al empezar el juego.", "The board's DIP switches; applied when the game starts.");
			add(R_FREE, K_TOGGLE, tr("Juego libre", "Free play"), tr("Sin monedas: Start empieza la partida. ", "No coins: Start begins a game. ") + h, "", m_cfg.dips.free_play);
			add(R_BLOOD, K_TOGGLE, tr("Sangre", "Blood"), h, "", m_cfg.dips.blood);
			add(R_VIOLENCE, K_TOGGLE, tr("Violencia (fatalities)", "Violence (fatalities)"), h, "", m_cfg.dips.violence);
			add(R_LOWBLOWS, K_TOGGLE, tr("Golpes bajos (MK, MK II)", "Low blows (MK, MK II)"), h, "", m_cfg.dips.low_blows);
			add(R_ATTRACT, K_TOGGLE, tr("Sonido en la demo", "Attract mode sound"), h, "", m_cfg.dips.attract_sound);
			add(R_COUNTRY, K_CHOICE, tr("Pais", "Country"), h, country_name(m_cfg.dips.country));
			break;
		}
		case P_SYSTEM:
		{
			add(R_LANG, K_CHOICE, "Idioma / Language", tr("Idioma de los menus.", "Language of the menus."),
			    m_cfg.lang < 0 ? tr("Automatico", "Automatic") : m_cfg.lang == 0 ? "Castellano" : "English");
			add(R_ROMDIR, K_CHOICE, tr("Carpeta de ROMs extra", "Extra ROM folder"),
			    tr("A / Intro para escribirla (Ctrl+V pega). La carpeta roms/ junto al programa se mira siempre; tambien subcarpetas y ZIP.",
			       "A / Enter to type it (Ctrl+V pastes). The roms/ folder next to the program is always searched; sub-folders and ZIPs too."),
			    m_cfg.rom_dir.empty() ? tr("(ninguna)", "(none)") : m_cfg.rom_dir);
			int found = 0, rc = 0;
			for (size_t i = 0; i < m_avail.size(); i++) { found += m_avail[i]; rc += m_has_rc[i]; }
			add(R_RESCAN, K_ACTION, tr("Volver a buscar las ROMs", "Search the ROMs again"), tr("Tras copiar ROMs o generar modulos recompilados.", "After copying ROMs or building recompiled modules."),
			    std::to_string(found) + " / " + std::to_string(m_avail.size()));
			add(R_RECOMP, K_INFO, tr("Juegos recompilados", "Recompiled games"),
			    tr("build_recomp.sh (Linux / SteamOS) o RECOMPILAR_RECOMPILE.bat (Windows) los generan desde tus ROMs.",
			       "build_recomp.sh (Linux / SteamOS) or RECOMPILAR_RECOMPILE.bat (Windows) build them from your ROMs."),
			    std::to_string(rc) + " / " + std::to_string(m_avail.size()));
			add(R_ABOUT, K_ACTION, tr("Acerca de Mortal Kombat Recomp", "About Mortal Kombat Recomp"), tr("Creditos y licencias.", "Credits and licences."));
			break;
		}
	}
	return r;
}

void app::row_change(const row &r, int d)
{
	int fwd = d ? d : 1;
	switch (r.id)
	{
		case R_SHADER: m_cfg.shader = (m_cfg.shader + fwd + kNumShaders) % kNumShaders; break;
		case R_SCAN: m_cfg.scan_strength = std::clamp(m_cfg.scan_strength + 0.1f * fwd, 0.0f, 1.0f); if (!d && m_cfg.scan_strength >= 0.99f) m_cfg.scan_strength = 0; break;
		case R_ASPECT: m_cfg.aspect = (m_cfg.aspect + fwd + 4) % 4; break;
		case R_INTEGER: m_cfg.integer_scale = d ? d > 0 : !m_cfg.integer_scale; break;
		case R_FULL: m_cfg.fullscreen = d ? d > 0 : !m_cfg.fullscreen; apply_fullscreen(); break;
		case R_VOLUME: m_cfg.volume = std::clamp(m_cfg.volume + 0.1f * fwd, 0.0f, 1.0f); if (!d && m_cfg.volume >= 0.99f) m_cfg.volume = 0; break;
		case R_PLAYER: m_ctrl_player = (m_ctrl_player + fwd + 2) % 2; break;
		case R_HK_MENU: m_cfg.hk_menu = (m_cfg.hk_menu + fwd + HKM_COUNT) % HKM_COUNT; break;
		case R_HK_SHADER: m_cfg.hk_shader = (m_cfg.hk_shader + fwd + HKS_COUNT) % HKS_COUNT; break;
		case R_TEST: m_test = d ? d > 0 : !m_test; break;
		case R_FREE: case R_BLOOD: case R_VIOLENCE: case R_LOWBLOWS: case R_ATTRACT: case R_COUNTRY:
		{
			dip_settings &s = m_cfg.dips;
			bool *b = r.id == R_FREE ? &s.free_play : r.id == R_BLOOD ? &s.blood : r.id == R_VIOLENCE ? &s.violence : r.id == R_LOWBLOWS ? &s.low_blows : r.id == R_ATTRACT ? &s.attract_sound : nullptr;
			if (b) *b = d ? d > 0 : !*b;
			else s.country = (s.country + fwd + 3) % 3;
			if (m_mach && !m_net_game) m_mach->set_dips(s);
			break;
		}
		case R_LANG: set_language((m_cfg.lang + 1 + fwd + 3) % 3 - 1); break;
		case R_CHEAT:
		{
			auto list = current_cheats();
			if (r.act >= 0 && size_t(r.act) < list.size())
			{
				const cheat_def &c = *list[size_t(r.act)];
				bool on = cheat_on(c);
				bool want = d ? d > 0 : !on;
				if (want != on || c.kind == cheat_kind::once) set_cheat(c, want);
			}
			break;
		}
		default: break;
	}
}

void app::row_activate(const row &r)
{
	switch (r.id)
	{
		case R_RESUME: close_menu(); break;
		case R_RESET: if (m_mach) m_mach->reset(); close_menu(); message("Reset", 1500); break;
		case R_SHOT: m_shot = true; close_menu(); break;
		case R_LAUNCHER: stop_game(); break;
		case R_QUIT: m_quit = true; break;
		case R_BIND:
			m_capture_act = r.act;
			m_capture_player = m_ctrl_player;
			m_capture_until = SDL_GetTicks() + 6000;
			break;
		case R_RESET_KEYS: m_cfg.reset_keys(); m_cfg.reset_pad(); message(tr("Controles restablecidos", "Controls reset"), 1800); break;
		case R_ROMDIR: m_edit_field = 2; m_edit = m_cfg.rom_dir; SDL_StartTextInput(); break;
		case R_RESCAN: refresh_availability(); message(tr("ROMs buscadas de nuevo", "ROMs searched again"), 1800); break;
		case R_ABOUT: m_screen = screen::about; break;
		case R_CHEAT: row_change(r, 0); break;
		default: if (r.kind != K_ACTION && r.kind != K_INFO) row_change(r, 0); break;
	}
}

void app::ui_capture_key(SDL_Scancode sc)
{
	if (sc == SDL_SCANCODE_ESCAPE) { m_capture_act = -1; return; }
	if (sc == SDL_SCANCODE_BACKSPACE || sc == SDL_SCANCODE_DELETE) sc = SDL_SCANCODE_UNKNOWN;   // clears the key
	m_cfg.keys[m_capture_player][m_capture_act] = sc;
	m_capture_act = -1;
}

void app::ui_capture_pad(int p)
{
	if (p == SDL_CONTROLLER_BUTTON_GUIDE) return;
	m_cfg.pad[m_capture_act][0] = p;
	m_cfg.pad[m_capture_act][1] = PAD_NONE;
	// the same input cannot do two things
	for (int a = 0; a < ACT_COUNT; a++)
		if (a != m_capture_act)
			for (int k = 0; k < 2; k++)
				if (m_cfg.pad[a][k] == p) m_cfg.pad[a][k] = PAD_NONE;
	m_capture_act = -1;
}

void app::open_pause_menu()
{
	m_menu_open = true;
	m_page = 0;
	m_row = 0;
	m_scroll = 0;
	if (m_audio) SDL_ClearQueuedAudio(m_audio);
}

void app::close_menu()
{
	m_menu_open = false;
	m_capture_act = -1;
	m_next_frame = 0;
}

// ---------------------------------------------------------------------------
// keys
// ---------------------------------------------------------------------------
void app::ui_key(int k)
{
	if (ui_capturing()) return;
	if (ui_editing())
	{
		if (k == UK_OK)
		{
			if (m_edit_field == 1) m_cfg.net_host = m_edit;
			else { m_cfg.rom_dir = m_edit; refresh_availability(); }
		}
		if (k == UK_OK || k == UK_BACK) { m_edit_field = 0; SDL_StopTextInput(); }
		return;
	}
	if (m_screen == screen::game && m_menu_open) { pages_key(k); return; }
	switch (m_screen)
	{
		case screen::launcher: launcher_key(k); break;
		case screen::settings: pages_key(k); break;
		case screen::net: net_key(k); break;
		case screen::about: if (k == UK_OK || k == UK_BACK) m_screen = m_menu_open ? screen::game : screen::settings; break;
		default: break;
	}
}

void app::launcher_key(int k)
{
	int ng = int(game_list().size()), n = ng + 4;      // games + online, settings, about, quit
	if (k == UK_UP) m_sel = (m_sel + n - 1) % n;
	if (k == UK_DOWN) m_sel = (m_sel + 1) % n;
	if (k == UK_BACK) { if (m_sel == n - 1) m_quit = true; else m_sel = n - 1; }
	if (k == UK_NET || (k == UK_OK && m_sel == ng))
	{
		m_screen = screen::net;
		m_net_row = 0;
		m_local_ips = net_local_ipv4();
		return;
	}
	if (k != UK_OK) return;
	if (m_sel < ng)
	{
		if (m_avail[size_t(m_sel)]) start_game(m_sel);
		else message(m_why[size_t(m_sel)], 6000);
	}
	else if (m_sel == ng + 1) { m_screen = screen::settings; m_page = 0; m_row = 0; m_scroll = 0; }
	else if (m_sel == ng + 2) m_screen = screen::about;
	else m_quit = true;
}

void app::pages_key(int k)
{
	auto pages = page_list();
	m_page = std::clamp(m_page, 0, int(pages.size()) - 1);
	auto rows = page_rows(pages[size_t(m_page)]);
	int n = int(rows.size());
	auto selectable = [&](int i) { return rows[size_t(i)].kind != K_INFO; };
	auto tab = [&](int d) { m_page = (m_page + d + int(pages.size())) % int(pages.size()); m_row = 0; m_scroll = 0; };
	if (m_row >= n) m_row = std::max(0, n - 1);
	switch (k)
	{
		case UK_UP: case UK_DOWN:
		{
			int d = k == UK_UP ? -1 : 1;
			for (int i = 0; i < n; i++) { m_row = (m_row + d + n) % n; if (selectable(m_row)) break; }
			break;
		}
		case UK_PREV: tab(-1); break;
		case UK_NEXT: tab(1); break;
		case UK_LEFT: case UK_RIGHT:
		{
			if (n == 0 || !selectable(m_row)) { tab(k == UK_LEFT ? -1 : 1); break; }
			const row &r = rows[size_t(m_row)];
			if (r.kind == K_ACTION || r.kind == K_BIND) tab(k == UK_LEFT ? -1 : 1);
			else if (r.id == R_ROMDIR) tab(k == UK_LEFT ? -1 : 1);
			else row_change(r, k == UK_LEFT ? -1 : 1);
			break;
		}
		case UK_OK:
			if (n && selectable(m_row)) row_activate(rows[size_t(m_row)]);
			break;
		case UK_BACK:
			if (m_menu_open) close_menu();
			else m_screen = screen::launcher;
			break;
	}
}

void app::net_key(int k)
{
	if (m_net.active())
	{
		if (k == UK_BACK) m_net.stop();
		else if (k == UK_OK && m_net.role() == netplay::mode::host)
		{
			const game_info &g = game_list()[size_t(std::min(m_sel, int(game_list().size()) - 1))];
			m_net_game = true;
			if (!start_game(std::min(m_sel, int(game_list().size()) - 1))) { m_net_game = false; m_net.stop(); return; }
			m_net.host_start(g.id, m_mach->fingerprint(), m_cfg.dips, m_mach->nvram(), m_cfg.net_delay);
			message(tr("Partida en red: ", "Online game: ") + std::to_string(m_net.players()) + tr(" jugadores", " players"), 3000);
		}
		return;
	}
	const int n = 5;
	if (k == UK_UP) m_net_row = (m_net_row + n - 1) % n;
	if (k == UK_DOWN) m_net_row = (m_net_row + 1) % n;
	if (k == UK_BACK) { m_screen = screen::launcher; return; }
	if ((k == UK_LEFT || k == UK_RIGHT) && m_net_row == 2) m_cfg.net_port = std::clamp(m_cfg.net_port + (k == UK_RIGHT ? 1 : -1), 1024, 65535);
	if ((k == UK_LEFT || k == UK_RIGHT) && m_net_row == 3) m_cfg.net_delay = std::clamp(m_cfg.net_delay + (k == UK_RIGHT ? 1 : -1), 1, 10);
	if (k != UK_OK) return;
	int gi = std::min(m_sel, int(game_list().size()) - 1);
	std::string err;
	if (m_net_row == 0)
	{
		if (!m_avail[size_t(gi)]) message(m_why[size_t(gi)], 6000);
		else if (!m_net.host(u16(m_cfg.net_port), err)) message(tr("Red: ", "Network: ") + err, 6000);
	}
	else if (m_net_row == 1)
	{
		if (m_cfg.net_host.empty()) { m_edit_field = 1; m_edit.clear(); SDL_StartTextInput(); }
		else if (!m_net.join(m_cfg.net_host, u16(m_cfg.net_port), err)) message(tr("Red: ", "Network: ") + err, 6000);
	}
	else if (m_net_row == 4) m_screen = screen::launcher;
	else if (m_net_row == 2 || m_net_row == 3) {}
}

// ---------------------------------------------------------------------------
// drawing
// ---------------------------------------------------------------------------
static void panel(ui_canvas &c, float x, float y, float w, float h)
{
	float S = c.S;
	c.fill(x + 6 * S, y + 10 * S, w, h, 18 * S, kBlack, 0.45f);
	c.fill_v(x, y, w, h, 16 * S, kPanelTop, 0.94f, kPanelBot, 0.97f);
	c.outline(x, y, w, h, 16 * S, 1.5f * S, kBlood, 0.35f);
	c.fill_h(x + 24 * S, y, w - 48 * S, 2.5f * S, 1 * S, kDeep, 0.0f, kBlood, 0.95f);
}

static void keycap(ui_canvas &c, float x, float y, const std::string &k, ucol col, float size, float *w_out = nullptr)
{
	float S = c.S, tw = c.text_w(F_BODY, size, k), w = tw + 20 * S, h = size + 10 * S;
	c.fill(x, y, w, h, 7 * S, kFrame, 1);
	c.outline(x, y, w, h, 7 * S, 1.2f * S, kBlood, 0.7f);
	c.text(F_BODY, size, x + w / 2, y + 3 * S, k, col, 1, 1);
	if (w_out) *w_out = w;
}

static void chip(ui_canvas &c, float &x, float y, const std::string &key, const std::string &label)
{
	float S = c.S, w;
	keycap(c, x, y, key, kBone, 22 * S, &w);
	float lw = c.text(F_BODY, 22 * S, x + w + 9 * S, y + 4 * S, label, kBone, 0.9f);
	x += w + 9 * S + lw + 26 * S;
}

void app::draw_footer(ui_canvas &c, bool tabs)
{
	float S = c.S, y = c.h - 50 * S;
	c.fill_v(0, y - 10 * S, float(c.w), 60 * S, 0, kBlack, 0, kBlack, 0.6f);
	bool pad = !m_pads.empty();
	float x = 36 * S;
	chip(c, x, y, pad ? "A" : tr("Intro", "Enter"), tr("Aceptar", "Accept"));
	chip(c, x, y, pad ? "B" : "Esc", tr("Volver", "Back"));
	if (tabs) { chip(c, x, y, pad ? "LB / RB" : "Tab", tr("Pestana", "Tab")); chip(c, x, y, pad ? tr("Izq / Der", "Left / Right") : "< >", tr("Cambiar", "Change")); }
	else if (m_screen == screen::launcher) chip(c, x, y, pad ? "Y" : "N", tr("Juego en red", "Online game"));
}

void app::draw_backdrop(ui_canvas &c, bool over_game)
{
	float S = c.S;
	if (over_game)
	{
		for (int y = 0; y < c.h; y++)
		{
			float k = float(y) / c.h;
			ucol col = umix(kBlack, { 40, 0, 4 }, k);
			float a = 0.62f + 0.2f * k;
			for (int x = 0; x < c.w; x++) c.blend(x, y, col, a);
		}
		return;
	}
	for (int y = 0; y < c.h; y++)
	{
		float k = float(y) / c.h;
		ucol col = umix({ 30, 3, 5 }, { 62, 4, 8 }, k);
		u32 px = 0xff000000u | (u32(col.r) << 16) | (u32(col.g) << 8) | u32(col.b);
		std::fill(c.px.begin() + long(y) * c.w, c.px.begin() + long(y + 1) * c.w, px);
	}
	// glow behind the title
	float gx = 300 * S, gy = 120 * S, R = 420 * S;
	for (int y = std::max(0, int(gy - R)); y < std::min(c.h, int(gy + R)); y++)
		for (int x = std::max(0, int(gx - R)); x < std::min(c.w, int(gx + R)); x++)
		{
			float d = std::sqrt((x - gx) * (x - gx) + (y - gy) * (y - gy)) / R;
			if (d < 1) c.blend(x, y, kBlood, 0.10f * (1 - d) * (1 - d));
		}
	// vignette at the sides
	for (int i = 0; i < int(60 * S); i++)
	{
		float a = 0.6f * (1 - i / (60 * S));
		for (int y = 0; y < c.h; y++) { c.blend(i, y, kBlack, a); c.blend(c.w - 1 - i, y, kBlack, a); }
	}
	// blood running down from the top: fixed columns, each drop runs, falls off, starts again
	u32 t = SDL_GetTicks();
	int n = std::max(8, int(c.w / (38 * S)));
	for (int i = 0; i < n; i++)
	{
		u32 h = u32(i + 1) * 2654435761u;
		float x = float(h % u32(c.w));
		float wd = (3 + (h >> 8) % 5) * S;
		u32 period = 6000 + (h >> 12) % 9000;
		u32 ph = (t + (h >> 4) % period) % period;
		float len = (c.h * 0.85f) * float(ph) / period * (0.4f + ((h >> 20) % 60) / 100.0f);
		float a = ph > period * 9 / 10 ? 0.42f * float(period - ph) / (period / 10.0f) : 0.42f;
		c.fill_v(x, -wd, wd, len + wd, wd / 2, kDeep, a, { 170, 8, 14 }, a);
		c.disc(x + wd / 2, len, wd * 0.9f, { 160, 6, 12 }, a);
		c.fill(x + wd * 0.25f, 0, std::max(1.0f, wd * 0.18f), std::max(0.0f, len - wd), 0, { 255, 120, 120 }, a * 0.18f);
	}
	// pool at the bottom
	c.fill_v(0, c.h - 70 * S, float(c.w), 70 * S, 0, kDeep, 0, { 70, 0, 4 }, 0.8f);
}

void app::draw_toast(ui_canvas &c)
{
	if (m_message.empty() || SDL_GetTicks() >= m_message_until) return;
	float S = c.S, size = 24 * S;
	std::string m = c.fit(F_BODY, size, m_message, c.w - 120 * S);
	float w = c.text_w(F_BODY, size, m) + 44 * S, h = 46 * S, x = (c.w - w) / 2, y = c.h - 120 * S;
	c.fill(x, y, w, h, 12 * S, { 26, 4, 6 }, 0.94f);
	c.outline(x, y, w, h, 12 * S, 1.5f * S, kBlood, 0.8f);
	c.text(F_BODY, size, c.w / 2.0f, y + 8 * S, m, kBone, 1, 1);
}

void app::draw_launcher(ui_canvas &c)
{
	float S = c.S, W = float(c.w);
	draw_backdrop(c, false);
	float x = 64 * S;
	c.text(F_BODY, 21 * S, x, 30 * S, tr("MIDWAY ARCADE  ·  RECOMPILACION ESTATICA", "MIDWAY ARCADE  ·  STATIC RECOMPILATION"), kDim, 0.95f, 0, 3 * S);
	c.text(F_TITLE, 62 * S, x + 4 * S, 64 * S, "MORTAL KOMBAT", kBlack, 0.7f);
	c.text(F_TITLE, 62 * S, x, 60 * S, "MORTAL KOMBAT", kBlood);
	c.text(F_HEAD, 44 * S, x + 2 * S, 128 * S, "RECOMP", kBone);
	c.fill_h(x, 190 * S, 560 * S, 3 * S, 1.5f * S, kBlood, 1, kBlood, 0);

	bool card = W >= 1040 * S;
	float listw = card ? std::min(640 * S, W - 520 * S) : W - 2 * x;
	const auto &list = game_list();
	int ng = int(list.size());
	float y = 222 * S;
	u32 t = SDL_GetTicks();
	float pulse = 0.75f + 0.25f * std::sin(t / 250.0f);
	for (int i = 0; i < ng; i++, y += 62 * S)
	{
		const game_info &g = list[size_t(i)];
		bool s = i == m_sel, ok = m_avail[size_t(i)];
		if (s)
		{
			c.fill_h(x - 22 * S, y - 4 * S, listw + 22 * S, 58 * S, 10 * S, kBlood, 0.42f * pulse, kBlood, 0.02f);
			c.fill(x - 22 * S, y - 4 * S, 6 * S, 58 * S, 3 * S, kBlood, 1);
		}
		ucol nc = !ok ? ucol{ 120, 90, 86 } : s ? kWhite : kBone;
		c.text(F_HEAD, 31 * S, x, y - 2 * S, g.title, nc);
		c.text(F_BODY, 18 * S, x, y + 32 * S, c.fit(F_BODY, 18 * S, g_lang_en ? g.subtitle_en : g.subtitle, listw - 180 * S), ok ? kDim : ucol{ 110, 80, 76 });
		float bx = x + listw - 8 * S;
		float hw = c.text(F_BODY, 18 * S, bx, y + 4 * S, g.hardware, ok ? ucol{ 220, 120, 110 } : ucol{ 110, 80, 76 }, 1, 2);
		if (m_has_rc[size_t(i)])
		{
			float pw = 54 * S, px = bx - hw - 14 * S - pw;
			c.fill(px, y + 2 * S, pw, 26 * S, 13 * S, ok ? kBlood : kFrame, 1);
			c.text(F_BODY, 18 * S, px + pw / 2, y + 4 * S, "REC", kWhite, 1, 1);
		}
	}
	y += 8 * S;
	const char *items[4] = { tr("Juego en red", "Online game"), tr("Ajustes", "Settings"), tr("Acerca de", "About"), tr("Salir", "Quit") };
	for (int i = 0; i < 4; i++, y += 38 * S)
	{
		bool s = m_sel == ng + i;
		if (s) { c.fill_h(x - 22 * S, y - 2 * S, 360 * S, 36 * S, 8 * S, kBlood, 0.4f * pulse, kBlood, 0.0f); c.fill(x - 22 * S, y - 2 * S, 5 * S, 36 * S, 2.5f * S, kBlood, 1); }
		c.text(F_BODY, 26 * S, x, y, items[i], s ? kWhite : kBone, s ? 1 : 0.85f);
	}

	if (card)
	{
		float pw = 400 * S, ph = 330 * S, px = W - pw - 56 * S, py = 222 * S;
		panel(c, px, py, pw, ph);
		c.text(F_HEAD, 26 * S, px + 26 * S, py + 18 * S, tr("ESTADO", "STATUS"), kBlood, 1, 0, 2 * S);
		int found = 0, rc = 0;
		for (size_t i = 0; i < m_avail.size(); i++) { found += m_avail[i]; rc += m_has_rc[i]; }
		int gi = std::min(m_sel, ng - 1);
		bool gsel = m_sel < ng;
		struct line { std::string k, v; bool ok; } L[5] = {
			{ tr("Juegos", "Games"), std::to_string(found) + " / " + std::to_string(ng) + tr(" con ROMs", " with ROMs"), found > 0 },
			{ tr("Codigo", "Code"), gsel ? (m_has_rc[size_t(gi)] ? tr("recompilado (C++)", "recompiled (C++)") : tr("interprete", "interpreter"))
			                             : std::to_string(rc) + tr(" recompilados", " recompiled"), gsel ? bool(m_has_rc[size_t(gi)]) : rc > 0 },
			{ tr("Mando", "Gamepad"), m_pads.empty() ? tr("no conectado", "not connected") : SDL_GameControllerName(m_pads[0]) ? SDL_GameControllerName(m_pads[0]) : "?", !m_pads.empty() },
			{ tr("Video", "Video"), std::string(m_gl.ok() ? "OpenGL  ·  " : "SDL  ·  ") + shader_name(m_cfg.shader), m_gl.ok() },
			{ tr("Version", "Version"), app_version(), true },
		};
		float ly = py + 66 * S;
		for (auto &l : L)
		{
			c.disc(px + 32 * S, ly + 14 * S, 5 * S, l.ok ? kBlood : kDim, l.ok ? 1 : 0.6f);
			c.text(F_BODY, 22 * S, px + 48 * S, ly, l.k, kDim);
			c.text(F_BODY, 22 * S, px + pw - 24 * S, ly, c.fit(F_BODY, 22 * S, l.v, pw - 190 * S), kBone, 1, 2);
			ly += 38 * S;
		}
		if (gsel && !m_avail[size_t(gi)])
			c.text_wrap(F_BODY, 20 * S, px + 26 * S, ly + 6 * S, pw - 52 * S, m_why[size_t(gi)], kGold, 1);
		else if (gsel)
			c.text_wrap(F_BODY, 20 * S, px + 26 * S, ly + 6 * S, pw - 52 * S,
			            tr("A / Intro: jugar.  Durante la partida, Esc o Back+Start abren el menu.", "A / Enter: play.  While playing, Esc or Back+Start open the menu."), kDim, 1);
	}
	draw_footer(c, false);
}

void app::draw_pages(ui_canvas &c)
{
	float S = c.S, W = float(c.w);
	draw_backdrop(c, m_menu_open);
	float pw = std::min(W - 80 * S, 1120 * S), ph = 610 * S, px = (W - pw) / 2, py = 36 * S;
	panel(c, px, py, pw, ph);
	c.text(F_HEAD, 42 * S, px + 34 * S, py + 16 * S, m_menu_open ? tr("PAUSA", "PAUSED") : tr("AJUSTES", "SETTINGS"), kBlood);
	if (m_cur_game >= 0)
		c.text(F_BODY, 22 * S, px + pw - 34 * S, py + 30 * S, std::string(game_list()[size_t(m_cur_game)].title) + (m_version.empty() ? "" : "  ·  rev " + m_version), kDim, 1, 2);
	auto pages = page_list();
	m_page = std::clamp(m_page, 0, int(pages.size()) - 1);
	// tabs
	float tx = px + 34 * S, ty = py + 84 * S;
	for (int i = 0; i < int(pages.size()); i++)
	{
		const char *nm = page_name(pages[size_t(i)]);
		float w = c.text_w(F_HEAD, 24 * S, nm, 1 * S) + 34 * S;
		if (i == m_page)
		{
			c.fill_v(tx, ty, w, 42 * S, 9 * S, { 90, 14, 18 }, 0.6f, kFrame, 0.6f);
			c.fill_h(tx + 10 * S, ty + 39 * S, w - 20 * S, 3 * S, 1.5f * S, kBlood, 1, kBlood, 1);
		}
		c.text(F_HEAD, 24 * S, tx + 17 * S, ty + 7 * S, nm, i == m_page ? kWhite : kDim, 1, 0, 1 * S);
		tx += w + 8 * S;
	}
	c.hline(px + 34 * S, px + pw - 34 * S, ty + 44 * S, kBlood, 0.3f);
	// rows
	auto rows = page_rows(pages[size_t(m_page)]);
	int n = int(rows.size());
	if (m_row >= n) m_row = std::max(0, n - 1);
	while (n && rows[size_t(m_row)].kind == K_INFO && m_row + 1 < n) m_row++;
	float rx0 = px + 34 * S, rx1 = px + pw - 34 * S, rh = 47 * S, ry0 = ty + 58 * S;
	int vis = std::max(1, int((py + ph - 86 * S - ry0) / rh));
	if (m_row < m_scroll) m_scroll = m_row;
	if (m_row >= m_scroll + vis) m_scroll = m_row - vis + 1;
	m_scroll = std::clamp(m_scroll, 0, std::max(0, n - vis));
	u32 t = SDL_GetTicks();
	for (int i = m_scroll; i < std::min(n, m_scroll + vis); i++)
	{
		const row &r = rows[size_t(i)];
		float ry = ry0 + (i - m_scroll) * rh;
		bool s = i == m_row && r.kind != K_INFO;
		if (s)
		{
			c.fill_h(rx0, ry, rx1 - rx0, rh - 5 * S, 9 * S, kBlood, 0.26f, kBlood, 0.05f);
			c.fill(rx0, ry + 7 * S, 5 * S, rh - 19 * S, 2.5f * S, kBlood, 1);
		}
		ucol lc = r.kind == K_INFO ? kDim : s ? kWhite : kBone;
		c.text(F_BODY, 27 * S, rx0 + 20 * S, ry + 4 * S, r.label, lc);
		float vy = ry + 6 * S, vs = 24 * S;
		switch (r.kind)
		{
			case K_CHOICE:
			{
				std::string v = c.fit(F_BODY, vs, r.value, (rx1 - rx0) * 0.5f);
				float w = c.text(F_BODY, vs, rx1 - 46 * S, vy, v, s ? kGold : kBone, 1, 2);
				if (r.id != R_ROMDIR)
				{
					c.tri(rx1 - 22 * S, ry + 21 * S, 9 * S, 1, s ? kBlood : kDim, 1);
					c.tri(rx1 - 70 * S - w, ry + 21 * S, 9 * S, -1, s ? kBlood : kDim, 1);
				}
				break;
			}
			case K_TOGGLE:
			{
				float w = 70 * S, h = 32 * S, x = rx1 - 16 * S - w, y = ry + 5 * S;
				if (r.on) c.fill_h(x, y, w, h, h / 2, kDeep, 1, kBlood, 1);
				else { c.fill(x, y, w, h, h / 2, kFrame, 1); c.outline(x, y, w, h, h / 2, 1.5f * S, kDim, 0.5f); }
				c.disc(r.on ? x + w - h / 2 : x + h / 2, y + h / 2, h / 2 - 4 * S, r.on ? kWhite : kDim, 1);
				break;
			}
			case K_SLIDER:
			{
				float w = 280 * S, h = 10 * S, x = rx1 - 90 * S - w, y = ry + 17 * S;
				c.fill(x, y, w, h, h / 2, kFrame, 1);
				c.fill_h(x, y, w * r.frac, h, h / 2, kDeep, 1, kBlood, 1);
				c.disc(x + w * r.frac, y + h / 2, 12 * S, s ? kWhite : kBone, 1);
				c.text(F_BODY, vs, rx1 - 16 * S, vy, r.value, s ? kGold : kBone, 1, 2);
				break;
			}
			case K_BIND:
			{
				// keyboard key (selected player) and pad input, in two columns
				float size = 21 * S, wk = c.text_w(F_BODY, size, r.k1) + 20 * S, wp = c.text_w(F_BODY, size, r.k2) + 20 * S;
				float cp = rx1 - 120 * S, ck = rx1 - 360 * S;
				keycap(c, ck - wk / 2, ry + 5 * S, r.k1, s ? kGold : kBone, size);
				keycap(c, cp - wp / 2, ry + 5 * S, r.k2, s ? kGold : kBone, size);
				break;
			}
			case K_INFO:
				if (!r.value.empty()) c.text(F_BODY, vs, rx1 - 16 * S, vy, r.value, kBone, 1, 2);
				break;
			default:
				if (!r.value.empty()) c.text(F_BODY, vs, rx1 - 46 * S, vy, r.value, kDim, 1, 2);
				if (s) c.tri(rx1 - 24 * S, ry + 21 * S, 9 * S, 1, kBlood, 0.75f + 0.25f * std::sin(t / 250.0f));
				break;
		}
	}
	// scroll marks
	if (m_scroll > 0) c.tri_v(px + pw / 2, ry0 - 7 * S, 8 * S, -1, kBlood, 0.9f);
	if (m_scroll + vis < n) c.tri_v(px + pw / 2, ry0 + vis * rh + 3 * S, 8 * S, 1, kBlood, 0.9f);
	// help for the selected row
	c.hline(rx0, rx1, py + ph - 78 * S, kBlood, 0.25f);
	if (n) c.text_wrap(F_BODY, 21 * S, rx0 + 4 * S, py + ph - 70 * S, rx1 - rx0 - 8 * S, rows[size_t(m_row)].help, kDim, 1, 1.2f);
	// capture / edit boxes
	if (ui_capturing())
	{
		if (SDL_GetTicks() > m_capture_until) m_capture_act = -1;
		else
		{
			float bw = std::min(W - 120 * S, 760 * S), bh = 190 * S, bx = (W - bw) / 2, by = (c.h - bh) / 2;
			c.fill(0, 0, W, float(c.h), 0, kBlack, 0.45f);
			panel(c, bx, by, bw, bh);
			c.text(F_HEAD, 30 * S, bx + bw / 2, by + 24 * S, action_name(m_capture_act), kBlood, 1, 1);
			c.text(F_BODY, 24 * S, bx + bw / 2, by + 74 * S, std::string(tr("Pulsa una tecla (Jugador ", "Press a key (Player ")) + std::to_string(m_capture_player + 1) +
			       tr(") o un boton del mando", ") or a pad button"), kBone, 1, 1);
			c.text(F_BODY, 20 * S, bx + bw / 2, by + 118 * S, tr("Esc: cancelar   ·   Retroceso: quitar la tecla", "Esc: cancel   ·   Backspace: clear the key"), kDim, 1, 1);
			c.fill(bx + 40 * S, by + bh - 22 * S, (bw - 80 * S) * float(m_capture_until - SDL_GetTicks()) / 6000.0f, 4 * S, 2 * S, kBlood, 0.8f);
		}
	}
	draw_footer(c, true);
}

void app::draw_net(ui_canvas &c)
{
	float S = c.S, W = float(c.w);
	draw_backdrop(c, false);
	float pw = std::min(W - 80 * S, 1000 * S), ph = 610 * S, px = (W - pw) / 2, py = 36 * S;
	panel(c, px, py, pw, ph);
	int gi = std::min(m_sel, int(game_list().size()) - 1);
	c.text(F_HEAD, 42 * S, px + 34 * S, py + 16 * S, tr("JUEGO EN RED", "ONLINE GAME"), kBlood);
	c.text(F_HEAD, 26 * S, px + pw - 34 * S, py + 30 * S, game_list()[size_t(gi)].title, kBone, 1, 2);
	float x = px + 34 * S, y = py + 96 * S, x1 = px + pw - 34 * S;
	std::string ips;
	for (auto &s : m_local_ips) ips += (ips.empty() ? "" : ", ") + s;
	if (m_net.active())
	{
		bool host = m_net.role() == netplay::mode::host;
		auto line = [&](const std::string &s, ucol col = kBone) { y = c.text_wrap(F_BODY, 26 * S, x, y, x1 - x, s, col) + 4 * S; };
		if (host)
		{
			line(tr("Esperando jugadores. Dales tu IP y el puerto:", "Waiting for players. Give them your IP and port:"), kDim);
			line("IP: " + (ips.empty() ? std::string("?") : ips) + "    " + tr("puerto UDP ", "UDP port ") + std::to_string(m_cfg.net_port), kGold);
			line(tr("Retardo de entrada: ", "Input delay: ") + std::to_string(m_cfg.net_delay), kDim);
			y += 10 * S;
			line(tr("Jugadores: ", "Players: ") + std::to_string(m_net.players()) + " / 4");
			line(tr("J1  tu (anfitrion)", "P1  you (host)"), kDim);
			for (auto &p : m_net.peer_list()) line(p, kDim);
			y += 10 * S;
			line(tr("A / Intro: empezar (puedes empezar solo para probar)", "A / Enter: start (you can start alone to try it)"), kBone);
		}
		else
		{
			line(tr("Conectando con ", "Connecting to ") + m_cfg.net_host + ":" + std::to_string(m_cfg.net_port), kGold);
			if (m_net.local_slot() < 0) line(tr("Esperando respuesta", "Waiting for an answer") + std::string((SDL_GetTicks() / 400) % 4, '.'), kDim);
			else
			{
				line(tr("Conectado como jugador ", "Connected as player ") + std::to_string(m_net.local_slot() + 1) + "  (" + std::to_string(m_net.ping_ms()) + " ms)");
				line(tr("Jugadores: ", "Players: ") + std::to_string(m_net.players()), kDim);
				line(m_net.stage() == netplay::phase::transfer ? tr("Recibiendo los datos de la partida...", "Receiving the game data...")
				                                               : tr("Esperando a que el anfitrion empiece...", "Waiting for the host to start..."), kDim);
			}
			line(tr("Usas los controles del J1 / primer mando.", "You use the P1 keys / first pad."), kDim);
		}
		line(tr("B / Esc: cancelar", "B / Esc: cancel"), kDim);
	}
	else
	{
		std::string ip = m_edit_field == 1 ? m_edit + ((SDL_GetTicks() / 400) % 2 ? "_" : " ") : (m_cfg.net_host.empty() ? tr("(escribe la IP)", "(type the IP)") : m_cfg.net_host);
		const std::string opts[5] = {
			tr("Crear partida (anfitrion, J1)", "Host a game (host, P1)"),
			tr("Unirse a: ", "Join: ") + ip,
			tr("Puerto UDP: ", "UDP port: ") + std::to_string(m_cfg.net_port),
			tr("Retardo de entrada: ", "Input delay: ") + std::to_string(m_cfg.net_delay) + tr(" fotogramas", " frames"),
			tr("Volver", "Back"),
		};
		for (int i = 0; i < 5; i++, y += 50 * S)
		{
			bool s = i == m_net_row;
			if (s) { c.fill_h(x, y - 4 * S, x1 - x, 46 * S, 9 * S, kBlood, 0.26f, kBlood, 0.05f); c.fill(x, y + 4 * S, 5 * S, 30 * S, 2.5f * S, kBlood, 1); }
			c.text(F_BODY, 27 * S, x + 20 * S, y, c.fit(F_BODY, 27 * S, opts[i], x1 - x - 60 * S), s ? kWhite : kBone);
			if (s && (i == 2 || i == 3)) { c.tri(x1 - 22 * S, y + 17 * S, 9 * S, 1, kBlood, 1); }
		}
		y += 10 * S;
		y = c.text_wrap(F_BODY, 21 * S, x, y, x1 - x,
		                tr("El anfitrion elige el juego (el marcado en el lanzador) y abre el puerto UDP en su router, o usa una red virtual (Tailscale, ZeroTier, Radmin VPN...). "
		                   "Todos necesitan las mismas ROMs. Retardo 3-4 en LAN, 5-7 por internet.",
		                   "The host picks the game (the one highlighted in the launcher) and opens the UDP port on the router, or uses a virtual LAN (Tailscale, ZeroTier, Radmin VPN...). "
		                   "Everybody needs the same ROMs. Delay 3-4 on a LAN, 5-7 over the internet."), kDim, 1, 1.2f);
		c.text(F_BODY, 21 * S, x, y + 6 * S, tr("Tu IP local: ", "Your local IP: ") + (ips.empty() ? std::string("?") : ips), kGold);
	}
	draw_footer(c, false);
}

void app::draw_about(ui_canvas &c)
{
	float S = c.S, W = float(c.w);
	draw_backdrop(c, m_menu_open);
	float pw = std::min(W - 80 * S, 1000 * S), ph = 610 * S, px = (W - pw) / 2, py = 36 * S;
	panel(c, px, py, pw, ph);
	c.text(F_TITLE, 34 * S, px + pw / 2, py + 20 * S, "MORTAL KOMBAT", kBlood, 1, 1);
	c.text(F_HEAD, 26 * S, px + pw / 2, py + 66 * S, "RECOMP  ·  " + app_version(), kBone, 1, 1);
	float x = px + 40 * S, x1 = px + pw - 40 * S, y = py + 120 * S;
	auto line = [&](const std::string &s, ucol col = kBone) { y = c.text_wrap(F_BODY, 22 * S, x, y, x1 - x, s, col, 1, 1.25f) + 8 * S; };
	line(tr("Creado por Pocket_lucho con Claude (Anthropic).", "Created by Pocket_lucho with Claude (Anthropic)."), kGold);
	line(tr("La saga recreativa Mortal Kombat de Midway (MK, MK II, MK3 y UMK3) en una sola aplicacion, con las ROMs de cada usuario. "
	        "El programa TMS34010 de cada juego se recompila estaticamente a C++ desde esas ROMs (REC en el lanzador).",
	        "Midway's Mortal Kombat arcade saga (MK, MK II, MK3 and UMK3) in a single application, running each user's own ROMs. "
	        "Each game's TMS34010 program is statically recompiled to C++ from those ROMs (REC in the launcher)."));
	line(tr("Placas T-Unit y Wolf Unit, sonido ADPCM y DCS: nucleos de MAME (BSD-3) e ymfm. Base: NBA JAM Recomp. "
	        "Fuentes Nosifer, Metal Mania y Oswald (SIL OFL), stb_truetype.",
	        "T-Unit and Wolf Unit boards, ADPCM and DCS sound: MAME cores (BSD-3) and ymfm. Based on NBA JAM Recomp. "
	        "Nosifer, Metal Mania and Oswald fonts (SIL OFL), stb_truetype."), kDim);
	line("github.com/PocketLucho/MORTALKOMBAT-RECOMP", kDim);
	line(tr("Si quieres apoyarme ve a https://buymeacoffee.com/pocket_lucho", "If you want to support me, go to https://buymeacoffee.com/pocket_lucho"), kGold);
	line(tr("Mortal Kombat (c) Midway / Warner Bros. Proyecto de aficionados sin animo de lucro.", "Mortal Kombat (c) Midway / Warner Bros. Non-profit fan project."), kDim);
	draw_footer(c, false);
}

// small overlays while playing: online tag, network waits
static void draw_game_overlay(ui_canvas &c, netplay &net, bool net_game)
{
	if (!net_game) return;
	float S = c.S;
	std::string tag = tr("RED J", "NET P") + std::to_string(net.local_slot() + 1) + "  " + std::to_string(net.ping_ms()) + " ms";
	float w = c.text_w(F_BODY, 20 * S, tag) + 24 * S;
	c.fill(c.w - w - 12 * S, 12 * S, w, 34 * S, 9 * S, kBlack, 0.6f);
	c.text(F_BODY, 20 * S, c.w - 12 * S - w / 2, 17 * S, tag, { 140, 255, 140 }, 1, 1);
	if (net.desynced())
	{
		c.fill(0, 60 * S, float(c.w), 44 * S, 0, kDeep, 0.85f);
		c.text(F_BODY, 24 * S, c.w / 2.0f, 66 * S, tr("DESINCRONIZACION: reinicia la partida en red", "DESYNC: restart the online game"), kWhite, 1, 1);
	}
	if (net.stall_ms() > 400) c.text(F_BODY, 28 * S, c.w / 2.0f, c.h / 2.0f, tr("Esperando a la red...", "Waiting for the network..."), kGold, 1, 1);
}

void app::ui_draw(ui_canvas &c)
{
	int ww = 1280, wh = 720;
	if (m_gl.ok()) SDL_GL_GetDrawableSize(m_win, &ww, &wh);
	else if (m_ren) SDL_GetRendererOutputSize(m_ren, &ww, &wh);
	ww = std::max(ww, 320); wh = std::max(wh, 240);
	// at most 900 lines: drawn in software every frame
	int h = std::min(wh, 900), w = int(int64_t(ww) * h / wh);
	c.begin(w, h);
	if (m_screen == screen::game && !m_menu_open) draw_game_overlay(c, m_net, m_net_game);
	else if (m_screen == screen::about) draw_about(c);
	else if (m_screen == screen::launcher) draw_launcher(c);
	else if (m_screen == screen::net) draw_net(c);
	else draw_pages(c);
	if (ui_editing() && m_edit_field == 2)
	{
		float S = c.S, bw = std::min(float(c.w) - 100 * S, 900 * S), bh = 170 * S, bx = (c.w - bw) / 2, by = (c.h - bh) / 2;
		c.fill(0, 0, float(c.w), float(c.h), 0, kBlack, 0.45f);
		panel(c, bx, by, bw, bh);
		c.text(F_HEAD, 28 * S, bx + 28 * S, by + 20 * S, tr("CARPETA DE ROMS", "ROM FOLDER"), kBlood);
		std::string v = m_edit + ((SDL_GetTicks() / 400) % 2 ? "_" : " ");
		c.fill(bx + 28 * S, by + 66 * S, bw - 56 * S, 44 * S, 8 * S, kFrame, 1);
		c.text(F_BODY, 24 * S, bx + 40 * S, by + 72 * S, v.size() > 2 && c.text_w(F_BODY, 24 * S, v) > bw - 90 * S ? "..." + v.substr(v.size() - 60) : v, kBone);
		c.text(F_BODY, 19 * S, bx + 28 * S, by + 122 * S, tr("Intro: aceptar   Esc: cancelar   Ctrl+V: pegar   (vacia = solo roms/)", "Enter: accept   Esc: cancel   Ctrl+V: paste   (empty = roms/ only)"), kDim);
	}
	draw_toast(c);
}
