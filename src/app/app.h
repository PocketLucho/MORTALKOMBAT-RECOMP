// app.h - Mortal Kombat Recomp application: configuration, input mapping, the game loop
// (main.cpp) and the menus (menu.cpp).
#pragma once
#include "games.h"
#include "cheats.h"
#include "gl_present.h"
#include "netplay.h"
#include "ui_draw.h"

#include <SDL.h>
#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <vector>

// arcade panel actions
enum { ACT_UP, ACT_DOWN, ACT_LEFT, ACT_RIGHT, ACT_HP, ACT_BL, ACT_HK, ACT_LP, ACT_LK, ACT_B6, ACT_START, ACT_COIN, ACT_COUNT };
// pad inputs: SDL_GameControllerButton values, plus the triggers
enum { PAD_NONE = -1, PAD_LT = 100, PAD_RT = 101 };
// pad shortcuts
enum { HKM_BACK_START, HKM_GUIDE, HKM_L3_R3, HKM_COUNT };
enum { HKS_NONE, HKS_R3, HKS_L3, HKS_BACK_RB, HKS_BACK_LB, HKS_COUNT };

struct config
{
	int scale = 3;
	bool fullscreen = false;
	int shader = 3;               // 0 sharp, 1 smooth, 2 scanlines, 3 CRT, 4 CRT curved
	bool integer_scale = true;
	float scan_strength = 0.6f;
	int aspect = 0;               // 0 = 4:3, 1 = 16:9, 2 = 16:10, 3 = fill window
	bool use_opengl = true;
	float volume = 0.8f;
	int net_port = 7845;
	int net_delay = 4;
	int lang = -1;                // -1 = system language, 0 = Spanish, 1 = English
	std::string net_host;
	std::string rom_dir;          // extra ROM folder (besides roms/ next to the program)
	dip_settings dips;
	SDL_Scancode keys[4][ACT_COUNT];
	int pad[ACT_COUNT][2];        // two pad inputs per action, shared by every pad
	int hk_menu = HKM_BACK_START;
	int hk_shader = HKS_R3;
	std::map<std::string, std::string> cheats;   // game id -> "id,id,..." of the cheats held on
	config() { reset_keys(); reset_pad(); }
	void reset_keys();
	void reset_pad();
};

const char *action_name(int a);          // localised
const char *pad_input_name(int p);       // "A", "RB", "LT"...
const char *shader_name(int i);
std::string app_version();

class app
{
public:
	int run();
	std::string m_start_game;            // --game <id>

	// ---- menu.cpp
	enum class screen { launcher, settings, net, about, game };
	enum { UK_UP, UK_DOWN, UK_LEFT, UK_RIGHT, UK_OK, UK_BACK, UK_PREV, UK_NEXT, UK_NET };
	void ui_key(int k);
	void ui_draw(ui_canvas &c);
	bool ui_capturing() const { return m_capture_act >= 0; }
	void ui_capture_key(SDL_Scancode sc);
	void ui_capture_pad(int p);
	bool ui_editing() const { return m_edit_field != 0; }
	void open_pause_menu();
	void close_menu();

private:
	// ---- main.cpp
	void open_window();
	void present(const u32 *game_px, int w, int h, const ui_canvas *ui);
	void frame();                         // one pass of the main loop
	bool game_step();                     // runs one emulated frame if due; true when it ran
	bool start_game(int index, bool net_client = false);
	void stop_game();
	void poll_events();
	cabinet_inputs gather_inputs();
	float aspect_value() const { static const float a[4] = { 4.0f / 3.0f, 16.0f / 9.0f, 16.0f / 10.0f, 0.0f }; return a[std::clamp(m_cfg.aspect, 0, 3)]; }
	void save_screenshot(const u32 *px, int w, int h);
	void refresh_availability();
	void message(const std::string &m, u32 ms = 2500) { m_message = m; m_message_until = SDL_GetTicks() + ms; }
	std::string nv_path(const game_info &g) const { return path_join(path_join(m_base, "saves"), std::string(g.id) + ".nv"); }
	std::string rc_path(const game_info &g, const char *ext) const;
	void attach_recomp(const game_info &g);
	void save_recomp_misses(const game_info &g);
	void next_shader(int d);
	void apply_fullscreen();
	void set_language(int lang);
	bool pad_input_down(SDL_GameController *c, int p) const;
	void net_lobby_step();               // netplay lobby (host / client waiting)

	// ---- menu.cpp
	void draw_backdrop(ui_canvas &c, bool over_game);
	void draw_launcher(ui_canvas &c);
	void draw_pages(ui_canvas &c);
	void draw_net(ui_canvas &c);
	void draw_about(ui_canvas &c);
	void draw_toast(ui_canvas &c);
	void draw_footer(ui_canvas &c, bool tabs);
	void launcher_key(int k);
	void pages_key(int k);
	void net_key(int k);
	struct row;
	std::vector<row> page_rows(int page);
	std::vector<int> page_list() const;
	void row_change(const row &r, int dir);
	void row_activate(const row &r);
	std::vector<const cheat_def *> current_cheats();
	bool cheat_on(const cheat_def &c) const;
	void set_cheat(const cheat_def &c, bool on);

	std::string m_base, m_cfgpath;
	config m_cfg;
	SDL_Window *m_win = nullptr;
	SDL_Renderer *m_ren = nullptr;
	SDL_Texture *m_tex = nullptr, *m_uitex = nullptr;
	gl_presenter m_gl;
	int m_texw = 0, m_texh = 0, m_uiw = 0, m_uih = 0;
	SDL_AudioDeviceID m_audio = 0;
	std::vector<SDL_GameController *> m_pads;
	std::unique_ptr<game_catalog> m_cat;
	std::unique_ptr<arcade_machine> m_mach;
	std::string m_version;                // program revision of the running game
	screen m_screen = screen::launcher;
	bool m_menu_open = false;             // pause menu over the running game
	int m_cur_game = -1;
	int m_sel = 0;                        // launcher: selected entry (games, then the other items)
	bool m_quit = false, m_test = false, m_shot = false;
	std::string m_message;
	u32 m_message_until = 0;
	std::vector<bool> m_avail, m_has_rc;
	std::vector<std::string> m_why;
	std::map<std::string, void *> m_rc_libs;
	std::vector<float> m_abuf;
	ui_canvas m_ui;
	double m_next_frame = 0;              // seconds (SDL performance counter)

	// menu state
	int m_page = 0, m_row = 0, m_scroll = 0;
	int m_capture_act = -1, m_capture_player = 0;   // rebinding
	u32 m_capture_until = 0;
	int m_ctrl_player = 0;                // controls page: player shown
	int m_edit_field = 0;                 // 0 none, 1 IP, 2 ROM folder
	std::string m_edit;
	int m_stick_dir = -1;                 // menu navigation with the stick
	u32 m_stick_next = 0;

	// netplay
	netplay m_net;
	int m_net_row = 0;
	bool m_net_game = false;
	std::vector<std::string> m_local_ips;
};
