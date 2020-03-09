#ifndef DOOD_H
#define DOOD_H

#include <stdint.h>

#define _dood_comp_cast(x) ((uint8_t)((x) * 255.0f) & 0xFF)
#define _dood_lerp(a, b, t) ((1.0f - (t)) * (a) + (b) * (t))
#define _dood_clamp(a, mi, ma) ((a) >= (ma) ? (ma) : (a) <= (mi) ? (mi) : (a))
#define _dood_remap(v, fmi, foma, tmi, tma) ((tmi) + (((v) - (fmi)) / ((foma) - (fmi))) * ((tma) - (tmi)))

enum BlendMode {
	BM_OPAQUE = 0,
	BM_ALPHA_BLEND,
	BM_ADDITIVE,
	BM_MULTIPLY
};

typedef struct Color_t {
	float r, g, b, a;
} Color;

struct Canvas_t;
typedef struct Canvas_t* Canvas;

extern Canvas dood_canvas_new(int width, int height);
extern void dood_canvas_free(Canvas canvas);

extern void dood_canvas_clear(Canvas canvas);
extern void dood_canvas_put(Canvas canvas, int x, int y, float r, float g, float b, float a);
extern void dood_canvas_draw_point(Canvas canvas, int x, int y);
extern void dood_canvas_draw_line(Canvas canvas, int x1, int y1, int x2, int y2);
extern void dood_canvas_draw_rect(Canvas canvas, int x, int y, int w, int h);
extern void dood_canvas_fill_rect(Canvas canvas, int x, int y, int w, int h);

extern void dood_canvas_draw_canvas(
	Canvas dest, Canvas src,
	int sx, int sy, int sw, int sh,
	int dx, int dy, int dw, int dh
);

extern void dood_canvas_get(Canvas canvas, int x, int y, float* r, float* g, float* b, float* a);
extern void dood_canvas_get_size(Canvas canvas, int* w, int* h);

extern void dood_canvas_set_blend(Canvas canvas, int blendMode);
extern void dood_canvas_set_draw_color(Canvas canvas, float r, float g, float b, float a);
extern void dood_canvas_set_draw_color_gray(Canvas canvas, float v);

extern void dood_canvas_set_clip(Canvas canvas, int x, int y, int w, int h);
extern void dood_canvas_remove_clip(Canvas canvas);

extern void dood_canvas_save_ppm(Canvas canvas, const char* outFile);

extern uint8_t* dood_canvas_get_buffer(Canvas canvas, int drawBuffer);

#ifdef DOOD_IMPLEMENTATION

#include <stdlib.h>
#include <memory.h>

struct Canvas_t {
	int width, height;

	uint8_t* pixels;
	int clip[4];

	Color drawColor;
	int blendMode, drawBuffer;
};

Canvas dood_canvas_new(int width, int height) {
	Canvas cnv = (Canvas) malloc(sizeof(*cnv));
	cnv->width = width;
	cnv->height = height;
	cnv->blendMode = BM_OPAQUE;
	cnv->pixels = (uint8_t*) malloc(sizeof(uint8_t) * width * height * 4);
	dood_canvas_remove_clip(cnv);
	dood_canvas_set_draw_color(cnv, 0, 0, 0, 1.0f);
	return cnv;
}

Canvas dood_canvas_load_ppm(const char* fileName) {
	FILE* fp = fopen(fileName, "r");
	if (fp) {
		char header[3];
		fscanf(fp, "%s", header);
		if (header[1] != '3') {
			fclose(fp);
			return NULL;
		}

		int w, h, d;
		fscanf(fp, "%d %d", &w, &h);
		fscanf(fp, "%d", &d);

		Canvas cnv = dood_canvas_new(w, h);
		for (int i = 0; i < w * h; i++) {
			int r, g, b;
			fscanf(fp, "%d %d %d", &r, &g, &b);
			dood_canvas_put(cnv, i % w, i / w, (float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, 1.0f);
		}
		return cnv;

		fclose(fp);
	}
	return NULL;
}

void dood_canvas_free(Canvas canvas) {
	free(canvas->pixels);
	free(canvas);
}

void dood_canvas_put(Canvas canvas, int x, int y, float r, float g, float b, float a) {
	if (x < canvas->clip[0] || x >= canvas->clip[2] || y < canvas->clip[1] || y >= canvas->clip[3]) return;

	switch (canvas->blendMode) {
		default: break;
		case BM_MULTIPLY: {
			float pr, pg, pb, pa;
			dood_canvas_get(canvas, x, y, &pr, &pg, &pb, &pa);
			r *= pr;
			g *= pg;
			b *= pb;
			a *= pa;
		} break;
		case BM_ADDITIVE: {
			float pr, pg, pb, pa;
			dood_canvas_get(canvas, x, y, &pr, &pg, &pb, &pa);
			r += pr;
			g += pg;
			b += pb;
		} break;
		case BM_ALPHA_BLEND: {
			float pr, pg, pb, pa;
			dood_canvas_get(canvas, x, y, &pr, &pg, &pb, &pa);
			r = _dood_lerp(pr, r, a);
			g = _dood_lerp(pg, g, a);
			b = _dood_lerp(pb, b, a);
			a *= pa;
		} break;
	}

	r = _dood_clamp(r, 0.0f, 1.0f);
	g = _dood_clamp(g, 0.0f, 1.0f);
	b = _dood_clamp(b, 0.0f, 1.0f);
	a = _dood_clamp(a, 0.0f, 1.0f);

	uint8_t* buf = canvas->pixels;
	int i = (x + y * canvas->width) * 4;
	buf[i + 0] = _dood_comp_cast(r);
	buf[i + 1] = _dood_comp_cast(g);
	buf[i + 2] = _dood_comp_cast(b);
	buf[i + 3] = _dood_comp_cast(a);
}

void dood_canvas_draw_point(Canvas canvas, int x, int y) {
	dood_canvas_put(canvas, x, y, canvas->drawColor.r, canvas->drawColor.g, canvas->drawColor.b, canvas->drawColor.a);
}

void dood_canvas_draw_line(Canvas canvas, int x1, int y1, int x2, int y2) {
	int dx = abs(x2 - x1);
	int sx = x1 < x2 ? 1 : -1;
	int dy = -abs(y2 - y1);
	int sy = y1 < y2 ? 1 : -1;
	int err = dx + dy;
	int e2 = 0;

	int x = x1;
	int y = y1;

	while (1) {
		dood_canvas_draw_point(canvas, x, y);
		if (x == x2 && y == y2) break;
		e2 = 2 * err;
		if (e2 >= dy) { err += dy; x += sx; }
		if (e2 <= dx) { err += dx; y += sy; }
	}
}

void dood_canvas_draw_rect(Canvas canvas, int x, int y, int w, int h) {
	dood_canvas_draw_line(canvas, x, y, x + w, y);
	dood_canvas_draw_line(canvas, x + w, y, x + w, y + h);
	dood_canvas_draw_line(canvas, x, y + h, x + w, y + h);
	dood_canvas_draw_line(canvas, x, y, x, y + h);
}

void dood_canvas_fill_rect(Canvas canvas, int x, int y, int w, int h) {
	for (int py = y; py < y + h; py++) {
		for (int px = x; px < x + w; px++) {
			dood_canvas_draw_point(canvas, px, py);
		}
	}
}

void dood_canvas_get(Canvas canvas, int x, int y, float* r, float* g, float* b, float* a) {
	if (x < 0 || x >= canvas->width || y < 0 || y >= canvas->height) {
		return;
	}
	int i = (x + y * canvas->width) * 4;
	if (r) *r = (float) canvas->pixels[i + 0] / 255.0f;
	if (g) *g = (float) canvas->pixels[i + 1] / 255.0f;
	if (b) *b = (float) canvas->pixels[i + 2] / 255.0f;
	if (a) *a = (float) canvas->pixels[i + 3] / 255.0f;
}

void dood_canvas_get_size(Canvas canvas, int* w, int* h) {
	if (w) *w = canvas->width;
	if (h) *h = canvas->height;
}

void dood_canvas_set_draw_buffer(Canvas canvas, int drawBuffer) {
	canvas->drawBuffer = drawBuffer;
}

void dood_canvas_set_draw_color(Canvas canvas, float r, float g, float b, float a) {
	canvas->drawColor.r = r;
	canvas->drawColor.g = g;
	canvas->drawColor.b = b;
	canvas->drawColor.a = a;
}

void dood_canvas_set_draw_color_gray(Canvas canvas, float v) {
	dood_canvas_set_draw_color(canvas, v, v, v, 1.0f);
}

void dood_canvas_set_blend(Canvas canvas, int blendMode) {
	canvas->blendMode = blendMode;
}

void dood_canvas_set_clip(Canvas canvas, int x, int y, int w, int h) {
	if (x * y + (x+w) * (y+h) == 0) {
		dood_canvas_remove_clip(canvas);
		return;
	}
	canvas->clip[0] = x;
	canvas->clip[1] = y;
	canvas->clip[2] = x + w;
	canvas->clip[3] = x + h;
}

void dood_canvas_remove_clip(Canvas canvas) {
	dood_canvas_set_clip(canvas, 0, 0, canvas->width, canvas->height);
}

void dood_canvas_clear(Canvas canvas) {
	for (int i = 0; i < canvas->width * canvas->height * 4; i += 4) {
		canvas->pixels[i + 0] = _dood_comp_cast(canvas->drawColor.r);
		canvas->pixels[i + 1] = _dood_comp_cast(canvas->drawColor.g);
		canvas->pixels[i + 2] = _dood_comp_cast(canvas->drawColor.b);
		canvas->pixels[i + 3] = _dood_comp_cast(canvas->drawColor.a);
	}
}

void dood_canvas_draw_canvas(
	Canvas dest, Canvas src,
	int sx, int sy, int sw, int sh,
	int dx, int dy, int dw, int dh
) {
	for (int y = 0; y < dh; y++) {
		for (int x = 0; x < dw; x++) {
			int cx = (int) _dood_remap((float) x, 0.0f, (float)dw, 0.0f, (float)sw);
			int cy = (int) _dood_remap((float) y, 0.0f, (float)dh, 0.0f, (float)sh);
			float pr = 0.0f, pg = 0.0f, pb = 0.0f, pa = 0.0f;
			dood_canvas_get(src, cx + sx, cy + sy, &pr, &pg, &pb, &pa);
			dood_canvas_put(dest, x + dx, y + dy, pr, pg, pb, pa);
		}
	}
}

void dood_canvas_save_ppm(Canvas canvas, const char* outFile) {
	FILE* fp = fopen(outFile, "w");
	if (fp) {
		fprintf(fp, "P3\n");
		fprintf(fp, "%d %d\n", canvas->width, canvas->height);
		fprintf(fp, "255\n");
		for (int i = 0; i < canvas->width * canvas->height * 4; i += 4) {
			uint8_t r = canvas->pixels[i + 0];
			uint8_t g = canvas->pixels[i + 1];
			uint8_t b = canvas->pixels[i + 2];
			fprintf(fp, "%3d %3d %3d\n", r, g, b);
		}
		fclose(fp);
	}
}
#endif
#endif // DOOD_H