#ifndef DOOD_H
#define DOOD_H

#include <stdint.h>

#define _dood_comp_cast(x) ((uint8_t)((x) * 255.0f) & 0xFF)
#define _dood_lerp(a, b, t) ((1.0f - (t)) * (a) + (b) * (t))
#define _dood_clamp(a, mi, ma) ((a) >= (ma) ? (ma) : (a) <= (mi) ? (mi) : (a))

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
extern void dood_canvas_draw_point(Canvas canvas, int x, int y);

extern void dood_canvas_get(Canvas canvas, int x, int y, float* r, float* g, float* b, float* a);
extern void dood_canvas_get_size(Canvas canvas, int* w, int* h);

extern void dood_canvas_set_blend(Canvas canvas, int blendMode);
extern void dood_canvas_set_draw_color(Canvas canvas, float r, float g, float b, float a);

extern void dood_canvas_save_ppm(Canvas canvas, const char* outFile);

#ifdef DOOD_IMPLEMENTATION

#include <stdlib.h>
#include <memory.h>

struct Canvas_t {
	int width, height;
	uint8_t* pixels;

	Color drawColor;
	int blendMode;
};

Canvas dood_canvas_new(int width, int height) {
	Canvas cnv = (Canvas) malloc(sizeof(*cnv));
	cnv->width = width;
	cnv->height = height;
	cnv->blendMode = BM_OPAQUE;
	cnv->pixels = (uint8_t*) malloc(sizeof(uint8_t) * width * height * 4);
	dood_canvas_set_draw_color(cnv, 0, 0, 0, 1.0f);
	return cnv;
}

void dood_canvas_free(Canvas canvas) {
	free(canvas->pixels);
	free(canvas);
}

void dood_canvas_draw_point(Canvas canvas, int x, int y) {
	if (x < 0 || x >= canvas->width || y < 0 || y >= canvas->height) {
		return;
	}
	int i = (x + y * canvas->width) * 4;

	float r, g, b, a;
	r = canvas->drawColor.r;
	g = canvas->drawColor.g;
	b = canvas->drawColor.b;
	a = canvas->drawColor.a;
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

	canvas->pixels[i + 0] = _dood_comp_cast(r);
	canvas->pixels[i + 1] = _dood_comp_cast(g);
	canvas->pixels[i + 2] = _dood_comp_cast(b);
	canvas->pixels[i + 3] = _dood_comp_cast(a);
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

void dood_canvas_set_draw_color(Canvas canvas, float r, float g, float b, float a) {
	canvas->drawColor.r = r;
	canvas->drawColor.g = g;
	canvas->drawColor.b = b;
	canvas->drawColor.a = a;
}

void dood_canvas_set_blend(Canvas canvas, int blendMode) {
	canvas->blendMode = blendMode;
}

void dood_canvas_clear(Canvas canvas) {
	for (int i = 0; i < canvas->width * canvas->height * 4; i += 4) {
		canvas->pixels[i + 0] = _dood_comp_cast(canvas->drawColor.r);
		canvas->pixels[i + 1] = _dood_comp_cast(canvas->drawColor.g);
		canvas->pixels[i + 2] = _dood_comp_cast(canvas->drawColor.b);
		canvas->pixels[i + 3] = _dood_comp_cast(canvas->drawColor.a);
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