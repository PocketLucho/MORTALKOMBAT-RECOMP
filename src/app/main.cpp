// main.cpp - Mortal Kombat Recomp: one application for the Midway Mortal Kombat arcade saga
// (MK, MK II, MK3, UMK3). SDL2 video / audio / input, the game loop, configuration,
// per-game NVRAM and the statically recompiled program modules. The menus are in menu.cpp.
#include "app.h"
#include "recomp/rc_abi.h"
#include "cpu/tms34010/tms34010.h"
#include "lang.h"
#include "miniz.h"
#include "shaders.h"

#ifdef _WIN32
#define RC_EXT ".dll"
#else
#define RC_EXT ".so"
#endif

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <fstream>
#include <set>
#include <sstream>
#include <thread>
#ifndef _WIN32
#include <unistd.h>
#endif

// ---------------------------------------------------------------------------
// configuration (mk_recomp.ini next to the executable)
// ---------------------------------------------------------------------------
static const char *kActKeys[ACT_COUNT] = { "up", "down", "left", "right", "high_punch", "block", "high_kick", "low_punch", "low_kick", "block2_run", "start", "coin" };

void config::reset_keys()
{
	static const SDL_Scancode def[4][ACT_COUNT] = {
		// arcade panel: HP BL HK on the top row, LP BL2/RUN LK on the bottom row
		{ SDL_SCANCODE_UP, SDL_SCANCODE_DOWN, SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT, SDL_SCANCODE_A, SDL_SCANCODE_S, SDL_SCANCODE_D, SDL_SCANCODE_Z, SDL_SCANCODE_C, SDL_SCANCODE_X, SDL_SCANCODE_1, SDL_SCANCODE_5 },
		{ SDL_SCANCODE_I, SDL_SCANCODE_K, SDL_SCANCODE_J, SDL_SCANCODE_L, SDL_SCANCODE_R, SDL_SCANCODE_T, SDL_SCANCODE_Y, SDL_SCANCODE_F, SDL_SCANCODE_H, SDL_SCANCODE_G, SDL_SCANCODE_2, SDL_SCANCODE_6 },
		{ SDL_SCANCODE_KP_8, SDL_SCANCODE_KP_2, SDL_SCANCODE_KP_4, SDL_SCANCODE_KP_6, SDL_SCANCODE_KP_7, SDL_SCANCODE_KP_DIVIDE, SDL_SCANCODE_KP_9, SDL_SCANCODE_KP_1, SDL_SCANCODE_KP_3, SDL_SCANCODE_KP_0, SDL_SCANCODE_3, SDL_SCANCODE_7 },
		{ SDL_SCANCODE_UNKNOWN, SDL_SCANCODE_UNKNOWN, SDL_SCANCODE_UNKNOWN, SDL_SCANCODE_UNKNOWN, SDL_SCANCODE_UNKNOWN, SDL_SCANCODE_UNKNOWN, SDL_SCANCODE_UNKNOWN, SDL_SCANCODE_UNKNOWN, SDL_SCANCODE_UNKNOWN, SDL_SCANCODE_UNKNOWN, SDL_SCANCODE_4, SDL_SCANCODE_8 },
	};
	std::memcpy(keys, def, sizeof(keys));
}

void config::reset_pad()
{
	// Y high punch, X low punch, B high kick, A low kick, RB/RT block, LB/LT block 2 / run
	static const int def[ACT_COUNT][2] = {
		{ SDL_CONTROLLER_BUTTON_DPAD_UP, PAD_NONE }, { SDL_CONTROLLER_BUTTON_DPAD_DOWN, PAD_NONE },
		{ SDL_CONTROLLER_BUTTON_DPAD_LEFT, PAD_NONE }, { SDL_CONTROLLER_BUTTON_DPAD_RIGHT, PAD_NONE },
		{ SDL_CONTROLLER_BUTTON_Y, PAD_NONE }, { SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, PAD_RT }, { SDL_CONTROLLER_BUTTON_B, PAD_NONE },
		{ SDL_CONTROLLER_BUTTON_X, PAD_NONE }, { SDL_CONTROLLER_BUTTON_A, PAD_NONE }, { SDL_CONTROLLER_BUTTON_LEFTSHOULDER, PAD_LT },
		{ SDL_CONTROLLER_BUTTON_START, PAD_NONE }, { SDL_CONTROLLER_BUTTON_BACK, PAD_NONE },
	};
	std::memcpy(pad, def, sizeof(pad));
}

const char *action_name(int a)
{
	static const char *es[ACT_COUNT] = { "Arriba", "Abajo", "Izquierda", "Derecha", "Punetazo alto", "Bloqueo", "Patada alta", "Punetazo bajo",
	                                     "Patada baja", "Bloqueo 2 / correr", "Start", "Moneda" };
	static const char *en[ACT_COUNT] = { "Up", "Down", "Left", "Right", "High punch", "Block", "High kick", "Low punch",
	                                     "Low kick", "Block 2 / run", "Start", "Coin" };
	a = std::clamp(a, 0, ACT_COUNT - 1);
	return g_lang_en ? en[a] : es[a];
}

const char *pad_input_name(int p)
{
	switch (p)
	{
		case PAD_NONE: return "-";
		case PAD_LT: return "LT";
		case PAD_RT: return "RT";
		case SDL_CONTROLLER_BUTTON_A: return "A";
		case SDL_CONTROLLER_BUTTON_B: return "B";
		case SDL_CONTROLLER_BUTTON_X: return "X";
		case SDL_CONTROLLER_BUTTON_Y: return "Y";
		case SDL_CONTROLLER_BUTTON_BACK: return "Back";
		case SDL_CONTROLLER_BUTTON_GUIDE: return "Guide";
		case SDL_CONTROLLER_BUTTON_START: return "Start";
		case SDL_CONTROLLER_BUTTON_LEFTSTICK: return "L3";
		case SDL_CONTROLLER_BUTTON_RIGHTSTICK: return "R3";
		case SDL_CONTROLLER_BUTTON_LEFTSHOULDER: return "LB";
		case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: return "RB";
		case SDL_CONTROLLER_BUTTON_DPAD_UP: return g_lang_en ? "D-pad up" : "Cruceta arriba";
		case SDL_CONTROLLER_BUTTON_DPAD_DOWN: return g_lang_en ? "D-pad down" : "Cruceta abajo";
		case SDL_CONTROLLER_BUTTON_DPAD_LEFT: return g_lang_en ? "D-pad left" : "Cruceta izq.";
		case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: return g_lang_en ? "D-pad right" : "Cruceta der.";
		case SDL_CONTROLLER_BUTTON_MISC1: return "Share";
		case SDL_CONTROLLER_BUTTON_PADDLE1: return "P1";
		case SDL_CONTROLLER_BUTTON_PADDLE2: return "P2";
		case SDL_CONTROLLER_BUTTON_PADDLE3: return "P3";
		case SDL_CONTROLLER_BUTTON_PADDLE4: return "P4";
		case SDL_CONTROLLER_BUTTON_TOUCHPAD: return "Touchpad";
	}
	return "?";
}

static int pad_input_from_name(const std::string &n)
{
	for (int p = -1; p < SDL_CONTROLLER_BUTTON_MAX; p++)
		if (n == pad_input_name(p)) return p;
	if (n == "LT") return PAD_LT;
	if (n == "RT") return PAD_RT;
	int v = atoi(n.c_str());     // numeric form, language independent
	return (v >= 0 && v < SDL_CONTROLLER_BUTTON_MAX) || v == PAD_LT || v == PAD_RT ? v : PAD_NONE;
}

const char *shader_name(int i)
{
	static const char *es[] = { "Nitido (pixel perfect)", "Suavizado", "Scanlines", "CRT", "CRT curvo" };
	static const char *en[] = { "Sharp (pixel perfect)", "Smooth", "Scanlines", "CRT", "Curved CRT" };
	i = std::clamp(i, 0, kNumShaders - 1);
	return g_lang_en ? en[i] : es[i];
}

// version = build date (YYYY.MM.DD), taken from the compiler
std::string app_version()
{
	static const char *months = "JanFebMarAprMayJunJulAugSepOctNovDec";
	const char *d = __DATE__;            // "Mmm dd yyyy"
	int mon = int((strstr(months, std::string(d, 3).c_str()) - months) / 3) + 1;
	int day = atoi(d + 4), year = atoi(d + 7);
	char b[32];
	snprintf(b, sizeof(b), "%04d.%02d.%02d", year, mon, day);
	return b;
}

static std::string trim(std::string s)
{
	size_t a = s.find_first_not_of(" \t\r\n"), b = s.find_last_not_of(" \t\r\n");
	return a == std::string::npos ? std::string() : s.substr(a, b - a + 1);
}

static void save_config(const std::string &path, config &c)
{
	std::ofstream f(path);
	f << "# Mortal Kombat Recomp - configuration / configuracion (also from the menu: Esc / Back+Start)\n[general]\n"
	     "# language: es = castellano, en = English, auto\nlanguage="
	  << (c.lang == 1 ? "en" : c.lang == 0 ? "es" : "auto")
	  << "\n# extra folder with ROMs (roms/ next to the program is always searched)\nrom_dir=" << c.rom_dir
	  << "\n\n[video]\nscale=" << c.scale << "\nfullscreen=" << c.fullscreen
	  << "\n# shader: 0 nitido, 1 suavizado, 2 scanlines, 3 CRT, 4 CRT curvo\nshader=" << c.shader
	  << "\n# aspect: 0 = 4:3 original, 1 = 16:9, 2 = 16:10, 3 = llenar ventana\naspect=" << c.aspect << "\ninteger_scale=" << c.integer_scale
	  << "\nscanline_strength=" << c.scan_strength << "\n# opengl=0 usa el renderizador SDL basico (sin shaders)\nopengl=" << c.use_opengl
	  << "\n\n[audio]\nvolume=" << c.volume
	  << "\n\n[red]\nport=" << c.net_port << "\ninput_delay=" << c.net_delay << "\nlast_host=" << c.net_host
	  << "\n\n[dips]\nfree_play=" << c.dips.free_play << "\n# country: 0 USA, 1 French, 2 German\ncountry=" << c.dips.country
	  << "\nblood=" << c.dips.blood << "\nviolence=" << c.dips.violence << "\nlow_blows=" << c.dips.low_blows
	  << "\nattract_sound=" << c.dips.attract_sound
	  << "\n\n[cheats]\n";
	for (auto &[g, ids] : c.cheats) if (!ids.empty()) f << "cheats_" << g << "=" << ids << "\n";
	f << "\n[pad]\n# shared by every pad: SDL button names (A B X Y LB RB LT RT Back Start L3 R3 Guide...) or numbers\n";
	for (int i = 0; i < ACT_COUNT; i++)
		f << "pad_" << kActKeys[i] << "=" << c.pad[i][0] << "," << c.pad[i][1] << "\n";
	f << "# menu: 0 Back+Start, 1 Guide, 2 L3+R3   shader: 0 none, 1 R3, 2 L3, 3 Back+RB, 4 Back+LB\nhotkey_menu=" << c.hk_menu
	  << "\nhotkey_shader=" << c.hk_shader << "\n\n[keys]\n# nombres de tecla SDL (https://wiki.libsdl.org/SDL2/SDL_Scancode)\n";
	for (int p = 0; p < 4; p++)
		for (int i = 0; i < ACT_COUNT; i++)
			f << "p" << (p + 1) << "_" << kActKeys[i] << "=" << SDL_GetScancodeName(c.keys[p][i]) << "\n";
}

static void load_config(const std::string &path, config &c)
{
	std::ifstream f(path);
	if (!f) { save_config(path, c); return; }
	std::string line;
	while (std::getline(f, line))
	{
		line = trim(line);
		if (line.empty() || line[0] == '#' || line[0] == '[') continue;
		auto eq = line.find('=');
		if (eq == std::string::npos) continue;
		std::string k = trim(line.substr(0, eq)), v = trim(line.substr(eq + 1));
		int iv = atoi(v.c_str());
		if (k == "language") c.lang = v == "en" ? 1 : v == "es" ? 0 : -1;
		else if (k == "rom_dir") c.rom_dir = v;
		else if (k == "scale") c.scale = std::clamp(iv, 1, 8);
		else if (k == "fullscreen") c.fullscreen = iv != 0;
		else if (k == "shader") c.shader = std::clamp(iv, 0, kNumShaders - 1);
		else if (k == "aspect") c.aspect = std::clamp(iv, 0, 3);
		else if (k == "integer_scale") c.integer_scale = iv != 0;
		else if (k == "scanline_strength") c.scan_strength = std::clamp(float(atof(v.c_str())), 0.0f, 1.0f);
		else if (k == "opengl") c.use_opengl = iv != 0;
		else if (k == "port") c.net_port = std::clamp(iv, 1024, 65535);
		else if (k == "input_delay") c.net_delay = std::clamp(iv, 1, 10);
		else if (k == "last_host") c.net_host = v;
		else if (k == "volume") c.volume = std::clamp(float(atof(v.c_str())), 0.0f, 1.0f);
		else if (k == "free_play") c.dips.free_play = iv != 0;
		else if (k == "blood") c.dips.blood = iv != 0;
		else if (k == "violence") c.dips.violence = iv != 0;
		else if (k == "low_blows") c.dips.low_blows = iv != 0;
		else if (k == "attract_sound") c.dips.attract_sound = iv != 0;
		else if (k == "country") c.dips.country = std::clamp(iv, 0, 2);
		else if (k == "hotkey_menu") c.hk_menu = std::clamp(iv, 0, HKM_COUNT - 1);
		else if (k == "hotkey_shader") c.hk_shader = std::clamp(iv, 0, HKS_COUNT - 1);
		else if (k.rfind("cheats_", 0) == 0) c.cheats[k.substr(7)] = v;
		else if (k.rfind("pad_", 0) == 0)
		{
			for (int i = 0; i < ACT_COUNT; i++)
				if (k.substr(4) == kActKeys[i])
				{
					auto comma = v.find(',');
					c.pad[i][0] = pad_input_from_name(trim(v.substr(0, comma)));
					c.pad[i][1] = comma == std::string::npos ? PAD_NONE : pad_input_from_name(trim(v.substr(comma + 1)));
				}
		}
		else if (k.size() > 3 && k[0] == 'p' && k[2] == '_')
		{
			int p = k[1] - '1';
			if (p < 0 || p > 3) continue;
			for (int i = 0; i < ACT_COUNT; i++)
				if (k.substr(3) == kActKeys[i])
				{
					SDL_Scancode sc = SDL_GetScancodeFromName(v.c_str());
					if (sc != SDL_SCANCODE_UNKNOWN || v.empty()) c.keys[p][i] = sc;
				}
		}
	}
}

// ---------------------------------------------------------------------------
// statically recompiled programs: recomp/<game>.dll (Windows) or .so (Linux), built on the
// user's machine from their own ROMs (build_recomp.sh / RECOMPILAR_RECOMPILE.bat)
// ---------------------------------------------------------------------------
std::string app::rc_path(const game_info &g, const char *ext) const
{
	return path_join(path_join(m_base, "recomp"), std::string(g.id) + ext);
}

void app::attach_recomp(const game_info &g)
{
	std::string path = rc_path(g, RC_EXT);
	if (!file_exists(path)) return;
	void *lib = m_rc_libs.count(path) ? m_rc_libs[path] : nullptr;
	if (!lib)
	{
		lib = SDL_LoadObject(path.c_str());
		if (!lib) { std::fprintf(stderr, "recomp: %s\n", SDL_GetError()); message(tr("Modulo recompilado no valido: ", "Invalid recompiled module: ") + std::string(g.id) + RC_EXT, 5000); return; }
		m_rc_libs[path] = lib;
	}
	auto get = reinterpret_cast<rc_get_module_fn>(SDL_LoadFunction(lib, RC_ENTRY_NAME));
	std::string r = get ? m_mach->attach_recomp(get()) : "no entry point";
	std::fprintf(stderr, "recomp %s: %s\n", g.id, r.empty() ? "attached" : r.c_str());
	if (!r.empty())
		message(r == "ROM" ? tr("El modulo recompilado es de otras ROMs: vuelve a generarlo", "The recompiled module was built from other ROMs: build it again")
		                   : tr("Modulo recompilado de otra version del programa: vuelve a generarlo", "Recompiled module from another program version: build it again"), 6000);
}

// program addresses the module did not cover go to recomp/<game>.miss for the next build
void app::save_recomp_misses(const game_info &g)
{
	auto *c = m_mach ? dynamic_cast<tms34010_device *>(m_mach->maincpu()) : nullptr;
	if (!c || !c->recompiled() || c->rc_misses.empty()) return;
	std::string path = rc_path(g, ".miss");
	std::set<u32> all(c->rc_misses.begin(), c->rc_misses.end());
	std::vector<u8> old;
	if (read_whole_file(path, old))
	{
		std::istringstream in(std::string(old.begin(), old.end()));
		std::string line;
		while (std::getline(in, line)) if (!line.empty()) all.insert(u32(std::strtoul(line.c_str(), nullptr, 16)));
	}
	std::string txt;
	char b[16];
	for (u32 a : all) { std::snprintf(b, sizeof b, "%08x\n", a); txt += b; }
	write_whole_file(path, std::vector<u8>(txt.begin(), txt.end()));
}

// ---------------------------------------------------------------------------
void app::refresh_availability()
{
	m_cat = std::make_unique<game_catalog>(m_base, m_cfg.rom_dir);
	m_avail.clear();
	m_why.clear();
	m_has_rc.clear();
	for (const auto &g : game_list())
	{
		std::string why;
		m_avail.push_back(m_cat->available(g, why));
		m_why.push_back(why);
		m_has_rc.push_back(file_exists(rc_path(g, RC_EXT)));
	}
}

void app::set_language(int lang)
{
	m_cfg.lang = lang;
	if (lang < 0)
	{
		// follow the system language (Spanish -> castellano, anything else -> English)
		g_lang_en = true;
		if (SDL_Locale *loc = SDL_GetPreferredLocales())
		{
			if (loc[0].language && std::string(loc[0].language) == "es") g_lang_en = false;
			SDL_free(loc);
		}
	}
	else g_lang_en = lang == 1;
	if (m_cat) refresh_availability();
}

void app::apply_fullscreen()
{
	SDL_SetWindowFullscreen(m_win, m_cfg.fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
}

void app::next_shader(int d)
{
	m_cfg.shader = (m_cfg.shader + d + kNumShaders) % kNumShaders;
	message(std::string("Video: ") + shader_name(m_cfg.shader) + (m_gl.ok() ? "" : tr(" (sin OpenGL: aproximado)", " (no OpenGL: approximate)")), 1800);
}

void app::open_window()
{
	int w = 400 * m_cfg.scale, h = 300 * m_cfg.scale;
	Uint32 fs = m_cfg.fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0;
	if (m_cfg.use_opengl)
	{
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		m_win = SDL_CreateWindow("Mortal Kombat Recomp", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h,
			SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | fs);
		std::string err;
		if (m_win && m_gl.init(m_win, err)) return;
		if (m_win) SDL_DestroyWindow(m_win);
		m_win = nullptr;
		message(tr("OpenGL no disponible: sin shaders (", "OpenGL not available: no shaders (") + err.substr(0, 60) + ")", 8000);
		SDL_Log("OpenGL: %s", err.c_str());
	}
	m_win = SDL_CreateWindow("Mortal Kombat Recomp", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, SDL_WINDOW_RESIZABLE | fs);
	m_ren = SDL_CreateRenderer(m_win, -1, SDL_RENDERER_ACCELERATED);
	if (!m_ren) m_ren = SDL_CreateRenderer(m_win, -1, SDL_RENDERER_SOFTWARE);
}

// the game picture (with the shader) and the menu layer on top; either may be missing
void app::present(const u32 *px, int w, int h, const ui_canvas *ui)
{
	if (m_shot && ui && getenv("MK_SHOT_UI")) save_screenshot(ui->px.data(), ui->w, ui->h);   // tests: the menu layer
	else if (m_shot && px) save_screenshot(px, w, h);
	else if (m_shot && ui) save_screenshot(ui->px.data(), ui->w, ui->h);
	if (m_gl.ok())
	{
		m_gl.present(px, w, h, m_cfg.shader, m_cfg.integer_scale, m_cfg.scan_strength, aspect_value(),
		             ui ? ui->px.data() : nullptr, ui ? ui->w : 0, ui ? ui->h : 0);
		return;
	}
	int ww, wh;
	SDL_GetRendererOutputSize(m_ren, &ww, &wh);
	SDL_SetRenderDrawColor(m_ren, 0, 0, 0, 255);
	SDL_RenderClear(m_ren);
	if (px)
	{
		bool smooth = m_cfg.shader == 1;
		if (!m_tex || w != m_texw || h != m_texh)
		{
			if (m_tex) SDL_DestroyTexture(m_tex);
			SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, smooth ? "1" : "0");
			m_tex = SDL_CreateTexture(m_ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, w, h);
			m_texw = w; m_texh = h;
		}
		SDL_UpdateTexture(m_tex, nullptr, px, w * 4);
		SDL_Rect dst;
		output_rect(ww, wh, w, h, m_cfg.integer_scale, aspect_value(), dst.x, dst.y, dst.w, dst.h);
		SDL_RenderCopy(m_ren, m_tex, nullptr, &dst);
		if (m_cfg.shader >= 2 && dst.h >= h * 2)
		{
			SDL_SetRenderDrawBlendMode(m_ren, SDL_BLENDMODE_BLEND);
			SDL_SetRenderDrawColor(m_ren, 0, 0, 0, Uint8(140 * m_cfg.scan_strength));
			for (int y = 0; y < h; y++)
			{
				int yy = dst.y + (y * dst.h) / h + (dst.h / h) - 1;
				SDL_RenderDrawLine(m_ren, dst.x, yy, dst.x + dst.w, yy);
			}
		}
	}
	if (ui)
	{
		if (!m_uitex || ui->w != m_uiw || ui->h != m_uih)
		{
			if (m_uitex) SDL_DestroyTexture(m_uitex);
			SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
			m_uitex = SDL_CreateTexture(m_ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, ui->w, ui->h);
			// premultiplied alpha
			SDL_SetTextureBlendMode(m_uitex, SDL_ComposeCustomBlendMode(SDL_BLENDFACTOR_ONE, SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA, SDL_BLENDOPERATION_ADD,
			                                                            SDL_BLENDFACTOR_ONE, SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA, SDL_BLENDOPERATION_ADD));
			m_uiw = ui->w; m_uih = ui->h;
		}
		SDL_UpdateTexture(m_uitex, nullptr, ui->px.data(), ui->w * 4);
		SDL_RenderCopy(m_ren, m_uitex, nullptr, nullptr);
	}
	SDL_RenderPresent(m_ren);
}

void app::save_screenshot(const u32 *px, int w, int h)
{
	m_shot = false;
	std::vector<u8> rgb(size_t(w) * h * 3);
	for (size_t i = 0; i < size_t(w) * h; i++)
	{
		rgb[i * 3] = u8(px[i] >> 16); rgb[i * 3 + 1] = u8(px[i] >> 8); rgb[i * 3 + 2] = u8(px[i]);
	}
	size_t len = 0;
	void *png = tdefl_write_image_to_png_file_in_memory(rgb.data(), w, h, 3, &len);
	if (!png) { message(tr("No se pudo crear la captura", "Could not create the screenshot"), 3000); return; }
	std::time_t t = std::time(nullptr);
	char stamp[32];
	std::strftime(stamp, sizeof(stamp), "%Y%m%d_%H%M%S", std::localtime(&t));
	std::string name = std::string(m_cur_game >= 0 ? game_list()[size_t(m_cur_game)].id : "menu") + "_" + stamp;
	std::string dir = path_join(m_base, "screenshots"), path;
	for (int n = 0; n < 100; n++)
	{
		path = path_join(dir, name + (n ? "_" + std::to_string(n) : std::string()) + ".png");
		if (!file_exists(path)) break;
	}
	bool ok = write_whole_file(path, std::vector<u8>(static_cast<u8 *>(png), static_cast<u8 *>(png) + len));
	mz_free(png);
	message(ok ? std::string(tr("Captura guardada: screenshots/", "Screenshot saved: screenshots/")) + path.substr(dir.size() + 1)
	           : std::string(tr("No se pudo guardar ", "Could not save ")) + path, 3000);
}

// ---------------------------------------------------------------------------
// input
// ---------------------------------------------------------------------------
bool app::pad_input_down(SDL_GameController *c, int p) const
{
	if (p == PAD_NONE) return false;
	if (p == PAD_LT) return SDL_GameControllerGetAxis(c, SDL_CONTROLLER_AXIS_TRIGGERLEFT) > 12000;
	if (p == PAD_RT) return SDL_GameControllerGetAxis(c, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) > 12000;
	return SDL_GameControllerGetButton(c, SDL_GameControllerButton(p)) != 0;
}

void app::poll_events()
{
	SDL_Event e;
	bool in_game = m_screen == screen::game && !m_menu_open;
	while (SDL_PollEvent(&e))
	{
		switch (e.type)
		{
			case SDL_QUIT: m_quit = true; break;
			case SDL_CONTROLLERDEVICEADDED:
				if (SDL_GameController *c = SDL_GameControllerOpen(e.cdevice.which)) m_pads.push_back(c);
				break;
			case SDL_CONTROLLERDEVICEREMOVED:
				for (size_t i = 0; i < m_pads.size(); i++)
					if (SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(m_pads[i])) == e.cdevice.which)
					{
						SDL_GameControllerClose(m_pads[i]);
						m_pads.erase(m_pads.begin() + long(i));
						break;
					}
				break;
			case SDL_CONTROLLERAXISMOTION:
				if (ui_capturing() && e.caxis.value > 20000)
				{
					if (e.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT) ui_capture_pad(PAD_LT);
					if (e.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT) ui_capture_pad(PAD_RT);
				}
				break;
			case SDL_CONTROLLERBUTTONDOWN:
			{
				int b = e.cbutton.button;
				SDL_GameController *gc = SDL_GameControllerFromInstanceID(e.cbutton.which);
				auto held = [&](SDL_GameControllerButton x) { return gc && SDL_GameControllerGetButton(gc, x); };
				if (ui_capturing()) { ui_capture_pad(b); break; }
				// shortcuts (also while playing)
				bool menu_combo = (m_cfg.hk_menu == HKM_BACK_START && (b == SDL_CONTROLLER_BUTTON_START || b == SDL_CONTROLLER_BUTTON_BACK) &&
				                   held(SDL_CONTROLLER_BUTTON_START) && held(SDL_CONTROLLER_BUTTON_BACK)) ||
				                  (m_cfg.hk_menu == HKM_GUIDE && b == SDL_CONTROLLER_BUTTON_GUIDE) ||
				                  (m_cfg.hk_menu == HKM_L3_R3 && (b == SDL_CONTROLLER_BUTTON_LEFTSTICK || b == SDL_CONTROLLER_BUTTON_RIGHTSTICK) &&
				                   held(SDL_CONTROLLER_BUTTON_LEFTSTICK) && held(SDL_CONTROLLER_BUTTON_RIGHTSTICK));
				bool shader_combo = (m_cfg.hk_shader == HKS_R3 && b == SDL_CONTROLLER_BUTTON_RIGHTSTICK && !held(SDL_CONTROLLER_BUTTON_LEFTSTICK)) ||
				                    (m_cfg.hk_shader == HKS_L3 && b == SDL_CONTROLLER_BUTTON_LEFTSTICK && !held(SDL_CONTROLLER_BUTTON_RIGHTSTICK)) ||
				                    (m_cfg.hk_shader == HKS_BACK_RB && b == SDL_CONTROLLER_BUTTON_RIGHTSHOULDER && held(SDL_CONTROLLER_BUTTON_BACK)) ||
				                    (m_cfg.hk_shader == HKS_BACK_LB && b == SDL_CONTROLLER_BUTTON_LEFTSHOULDER && held(SDL_CONTROLLER_BUTTON_BACK));
				if (in_game)
				{
					if (menu_combo) open_pause_menu();
					else if (shader_combo) next_shader(1);
					break;
				}
				if (menu_combo && m_screen == screen::game) { close_menu(); break; }
				if (shader_combo && m_screen == screen::game) { next_shader(1); break; }
				// menu navigation
				switch (b)
				{
					case SDL_CONTROLLER_BUTTON_DPAD_UP: ui_key(UK_UP); break;
					case SDL_CONTROLLER_BUTTON_DPAD_DOWN: ui_key(UK_DOWN); break;
					case SDL_CONTROLLER_BUTTON_DPAD_LEFT: ui_key(UK_LEFT); break;
					case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: ui_key(UK_RIGHT); break;
					case SDL_CONTROLLER_BUTTON_A: ui_key(UK_OK); break;
					case SDL_CONTROLLER_BUTTON_START: if (!held(SDL_CONTROLLER_BUTTON_BACK)) ui_key(UK_OK); break;
					case SDL_CONTROLLER_BUTTON_B: case SDL_CONTROLLER_BUTTON_GUIDE: ui_key(UK_BACK); break;
					case SDL_CONTROLLER_BUTTON_LEFTSHOULDER: ui_key(UK_PREV); break;
					case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: ui_key(UK_NEXT); break;
					case SDL_CONTROLLER_BUTTON_Y: ui_key(UK_NET); break;
				}
				break;
			}
			case SDL_TEXTINPUT:
				if (ui_editing())
					for (const char *c = e.text.text; *c; c++)
						if (m_edit.size() < 200 && (unsigned char)*c >= 32) m_edit += *c;
				break;
			case SDL_KEYDOWN:
			{
				SDL_Keycode k = e.key.keysym.sym;
				bool alt = (e.key.keysym.mod & KMOD_ALT) != 0;
				if (ui_capturing()) { if (!e.key.repeat) ui_capture_key(e.key.keysym.scancode); break; }
				if (ui_editing())
				{
					if (k == SDLK_BACKSPACE && !m_edit.empty())
					{
						m_edit.pop_back();
						while (!m_edit.empty() && (static_cast<unsigned char>(m_edit.back()) & 0xc0) == 0x80) m_edit.pop_back();
					}
					if (k == SDLK_v && (e.key.keysym.mod & KMOD_CTRL))
						if (char *t = SDL_GetClipboardText()) { for (char *c = t; *c && m_edit.size() < 200; c++) if (*c != '\n' && *c != '\r') m_edit += *c; SDL_free(t); }
					if (k == SDLK_RETURN || k == SDLK_KP_ENTER) ui_key(UK_OK);
					if (k == SDLK_ESCAPE) ui_key(UK_BACK);
					break;
				}
				if (k == SDLK_F11 || (k == SDLK_RETURN && alt)) { m_cfg.fullscreen = !m_cfg.fullscreen; apply_fullscreen(); break; }
				if (k == SDLK_F12 || k == SDLK_PRINTSCREEN) { m_shot = true; break; }
				if (k == SDLK_F4) { next_shader((e.key.keysym.mod & KMOD_SHIFT) ? -1 : 1); break; }
				if (k == SDLK_F5) { m_cfg.integer_scale = !m_cfg.integer_scale; message(m_cfg.integer_scale ? tr("Escalado entero: SI", "Integer scaling: ON") : tr("Escalado entero: NO", "Integer scaling: OFF")); break; }
				if (k == SDLK_F7) { m_cfg.aspect = (m_cfg.aspect + 1) % 4; message(tr("Aspecto cambiado (menu > Video)", "Aspect changed (menu > Video)"), 1500); break; }
				if (k == SDLK_MINUS || k == SDLK_KP_MINUS) { m_cfg.volume = std::max(0.0f, m_cfg.volume - 0.1f); message(tr("Volumen ", "Volume ") + std::to_string(int(m_cfg.volume * 100 + 0.5f)) + "%", 1200); break; }
				if (k == SDLK_EQUALS || k == SDLK_PLUS || k == SDLK_KP_PLUS) { m_cfg.volume = std::min(1.0f, m_cfg.volume + 0.1f); message(tr("Volumen ", "Volume ") + std::to_string(int(m_cfg.volume * 100 + 0.5f)) + "%", 1200); break; }
				if (in_game)
				{
					if (k == SDLK_ESCAPE || k == SDLK_F1 || k == SDLK_p || k == SDLK_PAUSE) open_pause_menu();
					if (m_mach && !m_net_game)
					{
						if (k == SDLK_F2) { m_test = !m_test; message(m_test ? tr("Interruptor TEST: ON (menu de servicio)", "TEST switch: ON (service menu)") : tr("Interruptor TEST: OFF", "TEST switch: OFF"), 2000); }
						if (k == SDLK_F3) { m_mach->reset(); message("Reset", 1500); }
					}
					break;
				}
				if (k == SDLK_F1 && m_screen == screen::game) { close_menu(); break; }
				switch (k)
				{
					case SDLK_UP: ui_key(UK_UP); break;
					case SDLK_DOWN: ui_key(UK_DOWN); break;
					case SDLK_LEFT: ui_key(UK_LEFT); break;
					case SDLK_RIGHT: ui_key(UK_RIGHT); break;
					case SDLK_RETURN: case SDLK_KP_ENTER: case SDLK_SPACE: ui_key(UK_OK); break;
					case SDLK_ESCAPE: case SDLK_BACKSPACE: ui_key(UK_BACK); break;
					case SDLK_TAB: ui_key((e.key.keysym.mod & KMOD_SHIFT) ? UK_PREV : UK_NEXT); break;
					case SDLK_PAGEUP: case SDLK_q: ui_key(UK_PREV); break;
					case SDLK_PAGEDOWN: case SDLK_e: ui_key(UK_NEXT); break;
					case SDLK_n: ui_key(UK_NET); break;
				}
				break;
			}
		}
	}
	// menus: the left stick moves like the d-pad, with repeat
	if (!in_game && !ui_capturing())
	{
		int dir = -1;
		for (auto *c : m_pads)
		{
			int ax = SDL_GameControllerGetAxis(c, SDL_CONTROLLER_AXIS_LEFTX), ay = SDL_GameControllerGetAxis(c, SDL_CONTROLLER_AXIS_LEFTY);
			if (ay < -20000) dir = UK_UP;
			else if (ay > 20000) dir = UK_DOWN;
			else if (ax < -20000) dir = UK_LEFT;
			else if (ax > 20000) dir = UK_RIGHT;
		}
		u32 now = SDL_GetTicks();
		if (dir < 0) m_stick_dir = -1;
		else if (dir != m_stick_dir) { m_stick_dir = dir; m_stick_next = now + 380; ui_key(dir); }
		else if (now >= m_stick_next) { m_stick_next = now + 110; ui_key(dir); }
	}
}

cabinet_inputs app::gather_inputs()
{
	cabinet_inputs in{};
	if (m_menu_open) return in;          // the menu has the controls
	const Uint8 *ks = SDL_GetKeyboardState(nullptr);
	for (int p = 0; p < 4; p++)
	{
		bool v[ACT_COUNT] = {};
		for (int a = 0; a < ACT_COUNT; a++) v[a] = m_cfg.keys[p][a] != SDL_SCANCODE_UNKNOWN && ks[m_cfg.keys[p][a]];
		if (size_t(p) < m_pads.size())
		{
			SDL_GameController *c = m_pads[size_t(p)];
			for (int a = 0; a < ACT_COUNT; a++) v[a] = v[a] || pad_input_down(c, m_cfg.pad[a][0]) || pad_input_down(c, m_cfg.pad[a][1]);
			int ax = SDL_GameControllerGetAxis(c, SDL_CONTROLLER_AXIS_LEFTX), ay = SDL_GameControllerGetAxis(c, SDL_CONTROLLER_AXIS_LEFTY);
			v[ACT_UP] |= ay < -12000; v[ACT_DOWN] |= ay > 12000; v[ACT_LEFT] |= ax < -12000; v[ACT_RIGHT] |= ax > 12000;
			// the menu shortcut is not a coin + start
			if (m_cfg.hk_menu == HKM_BACK_START && SDL_GameControllerGetButton(c, SDL_CONTROLLER_BUTTON_BACK) && SDL_GameControllerGetButton(c, SDL_CONTROLLER_BUTTON_START))
				v[ACT_START] = v[ACT_COIN] = false;
		}
		auto &q = in.p[p];
		q.up = v[ACT_UP]; q.down = v[ACT_DOWN]; q.left = v[ACT_LEFT]; q.right = v[ACT_RIGHT];
		q.hp = v[ACT_HP]; q.bl = v[ACT_BL]; q.hk = v[ACT_HK]; q.lp = v[ACT_LP]; q.lk = v[ACT_LK]; q.b6 = v[ACT_B6];
		q.start = v[ACT_START]; q.coin = v[ACT_COIN];
	}
	in.test = m_test;
	in.service = ks[SDL_SCANCODE_9] != 0;
	return in;
}

// ---------------------------------------------------------------------------
// games
// ---------------------------------------------------------------------------
bool app::start_game(int index, bool net_client)
{
	const game_info &g = game_list()[size_t(index)];
	std::string err;
	SDL_SetWindowTitle(m_win, (std::string(tr("Mortal Kombat Recomp - cargando ", "Mortal Kombat Recomp - loading ")) + g.title + "...").c_str());
	m_mach = m_cat->create(g, err);
	if (!m_mach)
	{
		message("Error: " + err, 5000);
		SDL_SetWindowTitle(m_win, "Mortal Kombat Recomp");
		return false;
	}
	m_version = m_cat->version_label(g);
	if (!net_client)
	{
		std::vector<u8> nv;
		if (read_whole_file(nv_path(g), nv)) m_mach->set_nvram(nv);
		m_mach->set_dips(m_cfg.dips);
	}
	m_mach->reset();
	attach_recomp(g);
	m_cur_game = index;
	m_test = false;
	m_menu_open = false;
	m_screen = screen::game;
	m_next_frame = 0;
	SDL_SetWindowTitle(m_win, (std::string("Mortal Kombat Recomp - ") + g.title + (m_net_game ? tr("   [EN RED]", "   [ONLINE]") : "")).c_str());
	if (m_audio) SDL_ClearQueuedAudio(m_audio);
	if (!m_net_game)
		message(m_pads.empty() ? tr("Esc: menu (controles, video, trucos)", "Esc: menu (controls, video, cheats)")
		                       : tr("Esc o Back+Start: menu (controles, video, trucos)", "Esc or Back+Start: menu (controls, video, cheats)"), 3500);
	return true;
}

void app::stop_game()
{
	// a netplay client never overwrites its own NVRAM with the host's
	bool save = !(m_net_game && m_net.role() == netplay::mode::client);
	if (m_mach && m_cur_game >= 0 && save)
		write_whole_file(nv_path(game_list()[size_t(m_cur_game)]), m_mach->nvram());
	if (m_mach && m_cur_game >= 0) save_recomp_misses(game_list()[size_t(m_cur_game)]);
	m_mach.reset();
	m_cur_game = -1;
	m_menu_open = false;
	if (m_net_game)
	{
		std::string err = m_net.error();
		m_net.stop();
		m_net_game = false;
		if (!err.empty()) message(tr("Red: ", "Network: ") + err, 6000);
	}
	m_screen = screen::launcher;
	if (m_audio) SDL_ClearQueuedAudio(m_audio);
	SDL_SetWindowTitle(m_win, "Mortal Kombat Recomp");
	save_config(m_cfgpath, m_cfg);
}

// netplay lobby: the host starts the game, the client follows
void app::net_lobby_step()
{
	m_net.poll();
	if (!m_net.active()) return;
	bool host = m_net.role() == netplay::mode::host;
	if (host || !m_net.start_received()) return;
	int idx = -1;
	for (size_t i = 0; i < game_list().size(); i++) if (m_net.start_game_id() == game_list()[i].id) idx = int(i);
	std::string why;
	if (idx < 0) why = tr("juego desconocido: ", "unknown game: ") + m_net.start_game_id();
	else if (!m_avail[size_t(idx)]) why = std::string(tr("no tienes ", "you don't have ")) + game_list()[size_t(idx)].title;
	if (why.empty())
	{
		m_net_game = true;
		if (!start_game(idx, true)) why = tr("no se pudo cargar el juego", "the game could not be loaded");
		else if (m_mach->fingerprint() != m_net.start_fingerprint()) why = tr("tus ROMs no coinciden con las del anfitrion", "your ROMs are not the same as the host's");
	}
	if (!why.empty())
	{
		m_net.client_ready(false, why);
		m_net.poll();
		m_mach.reset();
		m_net_game = false;
		m_net.stop(why.c_str());
		message(tr("Red: ", "Network: ") + why, 6000);
		m_screen = screen::net;
		return;
	}
	m_mach->set_nvram(m_net.start_nvram());
	m_mach->set_dips(m_net.start_dips());
	m_mach->reset();
	m_net.client_ready(true, "");
}

// runs one emulated frame when it is due; true when it ran
bool app::game_step()
{
	bool ran = false;
	if (m_net_game)
	{
		m_net.poll();
		if (!m_net.active()) { stop_game(); return false; }
		if (m_net.can_run())
		{
			cabinet_inputs local = gather_inputs();
			cabinet_inputs in = m_net.frame_inputs(local.p[0]);
			m_mach->set_inputs(in);
			m_mach->run_frame();
			m_net.frame_done(m_mach->state_hash());
			ran = true;
		}
	}
	else if (!m_menu_open)
	{
		m_mach->set_inputs(gather_inputs());
		for (auto *c : current_cheats())
			if (c->kind == cheat_kind::hold && cheat_on(*c)) cheat_hold(*m_mach, *c);
		m_mach->run_frame();
		ran = true;
	}
	if (ran)
	{
		m_abuf.clear();
		m_mach->take_audio(m_abuf);
		if (m_audio && !m_abuf.empty())
		{
			for (float &v : m_abuf) v = std::clamp(v * m_cfg.volume * 1.6f, -1.0f, 1.0f);
			// keep latency bounded (about 120 ms)
			if (SDL_GetQueuedAudioSize(m_audio) < 48000 * 4 / 8)
				SDL_QueueAudio(m_audio, m_abuf.data(), Uint32(m_abuf.size() * sizeof(float)));
		}
	}
	return ran;
}

// ---------------------------------------------------------------------------
// main loop
// ---------------------------------------------------------------------------
void app::frame()
{
	poll_events();
	// MK_UI_SCRIPT: automated menu walk for tests, e.g. "size1280x800,wait30,shot,down,ok,wait90,shot,quit"
	if (static const char *script = getenv("MK_UI_SCRIPT"); script)
	{
		static std::vector<std::string> steps;
		static size_t pos = 0;
		static int wait = 0;
		if (steps.empty()) { std::stringstream ss(script); std::string t; while (std::getline(ss, t, ',')) steps.push_back(t); }
		if (wait > 0) wait--;
		else if (pos < steps.size())
		{
			std::string st = steps[pos++];
			static const std::map<std::string, int> keys = { { "up", UK_UP }, { "down", UK_DOWN }, { "left", UK_LEFT }, { "right", UK_RIGHT },
			                                                 { "ok", UK_OK }, { "back", UK_BACK }, { "prev", UK_PREV }, { "next", UK_NEXT }, { "net", UK_NET } };
			if (keys.count(st)) ui_key(keys.at(st));
			else if (st.rfind("wait", 0) == 0) wait = atoi(st.c_str() + 4);
			else if (st == "shot") m_shot = true;
			else if (st == "menu") open_pause_menu();
			else if (st == "quit") m_quit = true;
			else if (st.rfind("size", 0) == 0) { int w = atoi(st.c_str() + 4), h = atoi(st.c_str() + st.find('x') + 1); SDL_SetWindowSize(m_win, w, h); }
		}
	}
	if (m_quit) return;
	double now = double(SDL_GetPerformanceCounter()) / double(SDL_GetPerformanceFrequency());
	bool show_ui = m_screen != screen::game || m_menu_open || (!m_message.empty() && SDL_GetTicks() < m_message_until) || m_net_game;
	if (m_screen == screen::net && m_net.active()) net_lobby_step();
	if (m_screen == screen::game && m_mach)
	{
		bool ran = game_step();
		if (m_screen != screen::game || !m_mach) return;
		if (m_net_game && !ran)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			return;
		}
		if (ran || m_menu_open)
		{
			const ui_canvas *ui = nullptr;
			if (show_ui) { ui_draw(m_ui); ui = &m_ui; }
			present(m_mach->frame(), m_mach->width(), m_mach->height(), ui);
		}
		// pace to the game's own refresh rate (~54.7 Hz on these boards); menus at ~60 Hz
		double hz = m_menu_open && !m_net_game ? 60.0 : m_mach->refresh_hz();
		if (hz < 30 || hz > 120) hz = 60;
		if (m_next_frame <= 0) m_next_frame = now;
		m_next_frame += 1.0 / hz;
		double wait = m_next_frame - double(SDL_GetPerformanceCounter()) / double(SDL_GetPerformanceFrequency());
		bool behind = m_net_game && m_net.buffered() > m_net.delay() + 1;
		if (wait > 0 && !behind) std::this_thread::sleep_for(std::chrono::duration<double>(wait));
		else if (wait < -0.1) m_next_frame = now;
		return;
	}
	ui_draw(m_ui);
	present(nullptr, 0, 0, &m_ui);
	SDL_Delay(m_screen == screen::net ? 10 : 15);
}

int app::run()
{
	SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) != 0)
	{
		std::fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Mortal Kombat Recomp", SDL_GetError(), nullptr);
		return 1;
	}
	// no sound is not fatal (e.g. a system without a supported audio server)
	if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) std::fprintf(stderr, "audio: %s\n", SDL_GetError());
	std::fprintf(stderr, "Mortal Kombat Recomp %s - video: %s, audio: %s\n", app_version().c_str(), SDL_GetCurrentVideoDriver(),
	             SDL_GetCurrentAudioDriver() ? SDL_GetCurrentAudioDriver() : "none");
	char *bp = SDL_GetBasePath();
	m_base = bp ? bp : ".";
	SDL_free(bp);
	m_cfgpath = path_join(m_base, "mk_recomp.ini");
	load_config(m_cfgpath, m_cfg);
	set_language(m_cfg.lang);

	open_window();
	SDL_StopTextInput();
	SDL_AudioSpec want{}, have{};
	want.freq = 48000;
	want.format = AUDIO_F32SYS;
	want.channels = 1;
	want.samples = 1024;
	m_audio = SDL_OpenAudioDevice(nullptr, 0, &want, &have, 0);
	if (m_audio) SDL_PauseAudioDevice(m_audio, 0);
	else std::fprintf(stderr, "audio device: %s\n", SDL_GetError());

	refresh_availability();
	// start on the first available game
	for (size_t i = 0; i < m_avail.size(); i++) if (m_avail[i]) { m_sel = int(i); break; }
	// --game <id> (e.g. a Steam launch option): straight into that game
	for (size_t i = 0; i < game_list().size(); i++)
		if (m_start_game == game_list()[i].id && m_avail[i]) { m_sel = int(i); start_game(int(i)); }

	while (!m_quit) frame();

	if (m_screen == screen::game) stop_game();
	m_net.stop();
	save_config(m_cfgpath, m_cfg);
	for (auto *c : m_pads) SDL_GameControllerClose(c);
	if (m_audio) SDL_CloseAudioDevice(m_audio);
	if (m_tex) SDL_DestroyTexture(m_tex);
	if (m_uitex) SDL_DestroyTexture(m_uitex);
	m_gl.shutdown();
	if (m_ren) SDL_DestroyRenderer(m_ren);
	SDL_DestroyWindow(m_win);
	SDL_Quit();
	return 0;
}

int main(int argc, char **argv)
{
	SDL_SetMainReady();
#ifndef _WIN32
	// started from a file manager or Steam: keep the messages in a log next to the program
	if (!isatty(2))
		if (char *bp = SDL_GetBasePath())
		{
			std::string log = std::string(bp) + "mk_recomp.log";
			SDL_free(bp);
			if (std::freopen(log.c_str(), "w", stderr)) std::setvbuf(stderr, nullptr, _IOLBF, 0);
		}
#endif
	try
	{
		app a;
		for (int i = 1; i + 1 < argc; i++)
			if (std::string(argv[i]) == "--game") a.m_start_game = argv[i + 1];
		return a.run();
	}
	catch (const std::exception &e)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Mortal Kombat Recomp", e.what(), nullptr);
		return 1;
	}
}
