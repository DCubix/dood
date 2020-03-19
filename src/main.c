#include <stdio.h>
#include <math.h>

#define DOOD_IMPLEMENTATION
#include "dood.h"

#define BLUR_KERNEL 5
#define PI 3.141592654f

#define clamp(a, mi, ma) ((a) >= (ma) ? (ma) : (a) <= (mi) ? (mi) : (a))

float smoothstep(float e0, float e1, float x) {
	float t = clamp((x - e0) / (e1 - e0), 0.0f, 1.0f);
	return t * t * (3.0f - 2.0f * t);
}

Color blur(float x, float y, int cw, int ch, void* ud) {
	Canvas c = (Canvas) ud;

	int w, h;
	dood_canvas_get_size(c, &w, &h);

	int ix = x * w;
	int iy = y * h;

	int k = BLUR_KERNEL / 2;

	int div = 0;
	float r = 0.0f, g = 0.0f, b = 0.0f;
	for (int ky = -k; ky <= k; ky++) {
		for (int kx = -k; kx <= k; kx++) {
			float nr, ng, nb;
			dood_canvas_get(c, kx + ix, ky + iy, &nr, &ng, &nb, NULL);
			r += nr;
			g += ng;
			b += nb;
			div++;
		}
	}
	r /= div;
	g /= div;
	b /= div;

	return color(r, g, b, 1.0f);
}

// #define SWIRL_RADIUS 0.5f
// #define SWIRL_ANGLE (1.4f * PI)
// Color swirl(float x, float y, int cw, int ch, void* ud) {
// 	Canvas c = (Canvas) ud;

// 	int w, h;
// 	dood_canvas_get_size(c, &w, &h);

// 	float asp = (float)cw / (float)ch;
// 	float cx = x - 0.5f;
// 	float cy = y - 0.5f;

// 	float dx = cx * asp;
// 	float dy = cy * 1.0f;
// 	float len = sqrtf(dx * dx + dy * dy);
// 	float ang = atan2f(cy, cx) + SWIRL_ANGLE * smoothstep(SWIRL_RADIUS, 0.0f, len);
// 	float radius = sqrtf(cx * cx + cy * cy);

// 	float nr, ng, nb;
// 	dood_canvas_getf(
// 		c,
// 		(radius * cosf(ang) + 0.5f),
// 		(radius * sinf(ang) + 0.5f),
// 		&nr, &ng, &nb, NULL
// 	);

// 	return color(nr, ng, nb, 1.0f);
// }

#define COUNT 16

int main(int argc, char** argv) {
	Canvas cnv = dood_canvas_new(512, 512);
	dood_canvas_clear(cnv);

	int gap = cnv->height / (COUNT+2);
	int hh = gap * COUNT / 2;
	for (int i = 0; i <= COUNT; i++) {
		int y = (cnv->height / 2 - hh) + (i * gap);
		float f = ((float)i / COUNT) * 0.9f + 0.1f;
		float r = 0.2f;
		float g = 1.0f;
		float b = 0.5f;
		dood_canvas_set_draw_color(cnv, r * f, g * f, b * f, 1.0f);

		int py = 0.0f;
		for (int j = 32; j <= cnv->width - 32; j++) {
			float fac = (float)(j - 32) / (cnv->width - 64);
			fac *= PI * 2.0f;

			int yo = (int)(sinf(fac * 2.0f * i) * 8.0f);
			dood_canvas_draw_line(cnv, j, y + py, j + 1, y + yo);
			py = yo;
		}
	}

	dood_canvas_set_shader(cnv, &blur, cnv);

	dood_canvas_set_blend(cnv, BM_ADDITIVE);
	dood_canvas_fill_rect(cnv, 0, 0, cnv->width, cnv->height);
	dood_canvas_fill_rect(cnv, 0, 0, cnv->width, cnv->height);

	dood_canvas_save_bmp(cnv, "out.bmp");

	dood_canvas_free(cnv);
	return 0;
}
