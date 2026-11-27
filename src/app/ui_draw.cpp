// ui_draw.cpp - see ui_draw.h
#include "ui_draw.h"
#include "fonts_embed.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <map>
#include <memory>

#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC
#include "../third_party/stb/stb_truetype.h"

// ---------------------------------------------------------------------------
// fonts and glyph cache
// ---------------------------------------------------------------------------
namespace {
struct glyph { std::vector<uint8_t> bm; int w = 0, h = 0, xo = 0, yo = 0; float adv = 0; };
struct glyph_set
{
	float scale = 1;
	int asc = 0;
	std::map<int, glyph> g;
};
stbtt_fontinfo g_font[F_COUNT];
bool g_fonts_ok = false;
std::map<std::pair<int, int>, std::unique_ptr<glyph_set>> g_sets;

void fonts_init()
{
	if (g_fonts_ok) return;
	const unsigned char *d[F_COUNT] = { font_title, font_head, font_body };
	for (int i = 0; i < F_COUNT; i++) stbtt_InitFont(&g_font[i], d[i], stbtt_GetFontOffsetForIndex(d[i], 0));
	g_fonts_ok = true;
}
glyph_set &gset(int f, int px)
{
	fonts_init();
	auto &p = g_sets[{ f, px }];
	if (!p)
	{
		if (g_sets.size() > 48) { g_sets.clear(); return gset(f, px); }   // window resized many times
		p = std::make_unique<glyph_set>();
		p->scale = stbtt_ScaleForPixelHeight(&g_font[f], float(px));
		int asc, desc, gap;
		stbtt_GetFontVMetrics(&g_font[f], &asc, &desc, &gap);
		p->asc = int(asc * p->scale + 0.5f);
	}
	return *p;
}
const glyph &get_glyph(int f, glyph_set &s, int cp)
{
	auto it = s.g.find(cp);
	if (it != s.g.end()) return it->second;
	glyph g;
	int adv, lsb;
	stbtt_GetCodepointHMetrics(&g_font[f], cp, &adv, &lsb);
	g.adv = adv * s.scale;
	unsigned char *bm = stbtt_GetCodepointBitmap(&g_font[f], s.scale, s.scale, cp, &g.w, &g.h, &g.xo, &g.yo);
	if (bm) { g.bm.assign(bm, bm + size_t(g.w) * g.h); stbtt_FreeBitmap(bm, nullptr); }
	return s.g.emplace(cp, std::move(g)).first->second;
}
int utf8(const char *&p)
{
	const unsigned char *s = reinterpret_cast<const unsigned char *>(p);
	int c = s[0];
	if (c < 0x80) { p += 1; return c; }
	if ((c & 0xe0) == 0xc0 && s[1]) { p += 2; return ((c & 31) << 6) | (s[1] & 63); }
	if ((c & 0xf0) == 0xe0 && s[1] && s[2]) { p += 3; return ((c & 15) << 12) | ((s[1] & 63) << 6) | (s[2] & 63); }
	p += 1;
	return '?';
}
// coverage of a rounded rectangle at pixel centre (px, py)
float rr_cov(float px, float py, float x, float y, float w, float h, float rad)
{
	if (rad <= 0)
	{
		float e = std::min(std::min(px - x, x + w - px), std::min(py - y, y + h - py)) + 0.5f;
		return std::clamp(e, 0.0f, 1.0f);
	}
	float cx = std::clamp(px, x + rad, x + w - rad), cy = std::clamp(py, y + rad, y + h - rad);
	float dx = px - cx, dy = py - cy;
	if (dx == 0 && dy == 0)
	{
		float e = std::min(std::min(px - x, x + w - px), std::min(py - y, y + h - py)) + 0.5f;
		return std::clamp(e, 0.0f, 1.0f);
	}
	return std::clamp(rad - std::sqrt(dx * dx + dy * dy) + 0.5f, 0.0f, 1.0f);
}
} // namespace

// ---------------------------------------------------------------------------
void ui_canvas::begin(int W, int H)
{
	w = W; h = H;
	S = H / 720.0f;
	px.assign(size_t(W) * H, 0);
}

void ui_canvas::blend(int x, int y, ucol c, float a)
{
	if (x < 0 || y < 0 || x >= w || y >= h || a <= 0.002f) return;
	a = std::min(a, 1.0f);
	uint32_t &d = px[size_t(y) * w + x];
	float k = 1 - a;
	auto ch = [&](int sh, float v) { return uint32_t(std::min(255.0f, v * a + float((d >> sh) & 255) * k + 0.5f)) << sh; };
	d = ch(24, 255) | ch(16, c.r) | ch(8, c.g) | ch(0, c.b);
}

void ui_canvas::fill_v(float x, float y, float W, float H, float rad, ucol c0, float a0, ucol c1, float a1)
{
	int x0 = std::max(0, int(std::floor(x))), y0 = std::max(0, int(std::floor(y)));
	int x1 = std::min(w, int(std::ceil(x + W))), y1 = std::min(h, int(std::ceil(y + H)));
	for (int py = y0; py < y1; py++)
	{
		float t = H > 1 ? std::clamp((py + 0.5f - y) / H, 0.0f, 1.0f) : 0;
		ucol c = umix(c0, c1, t);
		float a = a0 + (a1 - a0) * t;
		for (int pxx = x0; pxx < x1; pxx++) blend(pxx, py, c, rr_cov(pxx + 0.5f, py + 0.5f, x, y, W, H, rad) * a);
	}
}

void ui_canvas::fill_h(float x, float y, float W, float H, float rad, ucol c0, float a0, ucol c1, float a1)
{
	int x0 = std::max(0, int(std::floor(x))), y0 = std::max(0, int(std::floor(y)));
	int x1 = std::min(w, int(std::ceil(x + W))), y1 = std::min(h, int(std::ceil(y + H)));
	for (int pxx = x0; pxx < x1; pxx++)
	{
		float t = W > 1 ? std::clamp((pxx + 0.5f - x) / W, 0.0f, 1.0f) : 0;
		ucol c = umix(c0, c1, t);
		float a = a0 + (a1 - a0) * t;
		for (int py = y0; py < y1; py++) blend(pxx, py, c, rr_cov(pxx + 0.5f, py + 0.5f, x, y, W, H, rad) * a);
	}
}

void ui_canvas::outline(float x, float y, float W, float H, float rad, float th, ucol c, float a)
{
	int x0 = std::max(0, int(std::floor(x))), y0 = std::max(0, int(std::floor(y)));
	int x1 = std::min(w, int(std::ceil(x + W))), y1 = std::min(h, int(std::ceil(y + H)));
	for (int py = y0; py < y1; py++)
		for (int pxx = x0; pxx < x1; pxx++)
		{
			float o = rr_cov(pxx + 0.5f, py + 0.5f, x, y, W, H, rad);
			if (o <= 0) continue;
			float i = rr_cov(pxx + 0.5f, py + 0.5f, x + th, y + th, W - 2 * th, H - 2 * th, std::max(0.0f, rad - th));
			blend(pxx, py, c, (o - i) * a);
		}
}

void ui_canvas::tri(float cx, float cy, float r, int dir, ucol c, float a)
{
	for (int y = int(cy - r) - 1; y <= int(cy + r) + 1; y++)
		for (int x = int(cx - r) - 1; x <= int(cx + r) + 1; x++)
		{
			float py = y + 0.5f - cy, pxx = (x + 0.5f - cx) * float(dir);
			if (pxx < -r * 0.8f - 0.5f || pxx > r + 0.5f) continue;
			float half = (r - pxx) * 0.5f;
			float cov = std::min(std::min(half - std::fabs(py) + 0.5f, pxx + r * 0.8f + 0.5f), 1.0f);
			if (cov > 0) blend(x, y, c, cov * a);
		}
}

void ui_canvas::tri_v(float cx, float cy, float r, int dir, ucol c, float a)
{
	for (int y = int(cy - r) - 1; y <= int(cy + r) + 1; y++)
		for (int x = int(cx - r) - 1; x <= int(cx + r) + 1; x++)
		{
			float px2 = x + 0.5f - cx, py = (y + 0.5f - cy) * float(dir);
			if (py < -r * 0.8f - 0.5f || py > r + 0.5f) continue;
			float half = (r - py) * 0.5f;
			float cov = std::min(std::min(half - std::fabs(px2) + 0.5f, py + r * 0.8f + 0.5f), 1.0f);
			if (cov > 0) blend(x, y, c, cov * a);
		}
}

void ui_canvas::hline(float x0, float x1, float y, ucol c, float a)
{
	for (int x = int(x0); x < int(x1); x++) blend(x, int(y), c, a);
}

float ui_canvas::text_w(int f, float size, const std::string &s, float track)
{
	glyph_set &g = gset(f, std::max(4, int(size + 0.5f)));
	float wd = 0;
	int n = 0;
	for (const char *p = s.c_str(); *p;) { wd += get_glyph(f, g, utf8(p)).adv + track; n++; }
	return n ? wd - track : 0;
}

float ui_canvas::text(int f, float size, float x, float y, const std::string &s, ucol c, float a, int align, float track)
{
	glyph_set &g = gset(f, std::max(4, int(size + 0.5f)));
	float wd = text_w(f, size, s, track);
	if (align == 1) x -= wd / 2;
	else if (align == 2) x -= wd;
	float pen = x;
	int base = int(y) + g.asc;
	for (const char *p = s.c_str(); *p;)
	{
		const glyph &gl = get_glyph(f, g, utf8(p));
		int ox = int(std::floor(pen + 0.5f)) + gl.xo, oy = base + gl.yo;
		for (int j = 0; j < gl.h; j++)
			for (int i = 0; i < gl.w; i++)
				if (uint8_t v = gl.bm[size_t(j) * gl.w + i]) blend(ox + i, oy + j, c, a * v / 255.0f);
		pen += gl.adv + track;
	}
	return wd;
}

std::string ui_canvas::fit(int f, float size, const std::string &s, float max_w)
{
	if (text_w(f, size, s) <= max_w) return s;
	std::string t = s;
	while (t.size() > 1)
	{
		t.pop_back();
		while (!t.empty() && (static_cast<unsigned char>(t.back()) & 0xc0) == 0x80) t.pop_back();   // whole UTF-8 characters
		if (text_w(f, size, t + "...") <= max_w) return t + "...";
	}
	return t;
}

float ui_canvas::text_wrap(int f, float size, float x, float y, float max_w, const std::string &s, ucol c, float a, float line)
{
	std::string cur, word;
	auto flush = [&]() { if (!cur.empty()) { text(f, size, x, y, cur, c, a); y += size * line; cur.clear(); } };
	for (size_t i = 0; i <= s.size(); i++)
	{
		char ch = i < s.size() ? s[i] : ' ';
		if (ch == '\n' || ch == ' ')
		{
			if (!word.empty())
			{
				std::string t = cur.empty() ? word : cur + " " + word;
				if (text_w(f, size, t) > max_w && !cur.empty()) { flush(); cur = word; }
				else cur = t;
				word.clear();
			}
			if (ch == '\n') flush();
		}
		else word += ch;
	}
	flush();
	return y;
}
