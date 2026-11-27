// ui_draw.h - drawing for the menus: a premultiplied ARGB canvas at (about) window
// resolution with anti-aliased rounded rectangles, gradients and TrueType text
// (stb_truetype, fonts under the SIL OFL in src/third_party/fonts).
#pragma once
#include <cstdint>
#include <string>
#include <vector>

struct ucol { float r, g, b; };
inline ucol umix(ucol a, ucol b, float t) { return { a.r + (b.r - a.r) * t, a.g + (b.g - a.g) * t, a.b + (b.b - a.b) * t }; }

enum ufont { F_TITLE, F_HEAD, F_BODY, F_COUNT };

class ui_canvas
{
public:
	int w = 0, h = 0;
	float S = 1;                     // layout unit: the layout is designed for 720 lines
	std::vector<uint32_t> px;        // premultiplied ARGB

	void begin(int W, int H);        // clears to transparent
	void blend(int x, int y, ucol c, float a);
	void fill_v(float x, float y, float w, float h, float rad, ucol c0, float a0, ucol c1, float a1);   // vertical gradient
	void fill_h(float x, float y, float w, float h, float rad, ucol c0, float a0, ucol c1, float a1);   // horizontal gradient
	void fill(float x, float y, float w, float h, float rad, ucol c, float a) { fill_v(x, y, w, h, rad, c, a, c, a); }
	void outline(float x, float y, float w, float h, float rad, float th, ucol c, float a);
	void disc(float cx, float cy, float r, ucol c, float a) { fill(cx - r, cy - r, 2 * r, 2 * r, r, c, a); }
	void tri(float cx, float cy, float r, int dir, ucol c, float a);     // dir -1 left, +1 right
	void tri_v(float cx, float cy, float r, int dir, ucol c, float a);   // dir -1 up, +1 down
	void hline(float x0, float x1, float y, ucol c, float a);
	// text: y = top of the line; align 0 left, 1 centre, 2 right; returns the width
	float text(int font, float px_size, float x, float y, const std::string &s, ucol c, float a = 1, int align = 0, float track = 0);
	float text_w(int font, float px_size, const std::string &s, float track = 0);
	// shortens s with "..." to fit max_w
	std::string fit(int font, float px_size, const std::string &s, float max_w);
	// word-wrapped text; returns the y below the last line
	float text_wrap(int font, float px_size, float x, float y, float max_w, const std::string &s, ucol c, float a = 1, float line = 1.25f);
};
