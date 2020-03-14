#include <stdio.h>
#include <math.h>

#define DOOD_IMPLEMENTATION
#include "dood.h"

#define BLUR_KERNEL 13

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

int main(int argc, char** argv) {
	Canvas ok = dood_canvas_load_bmp("eu.bmp");
	Canvas cnv = dood_canvas_new(ok->width, ok->height);
	dood_canvas_clear(cnv);
	
	dood_canvas_set_shader(cnv, &blur, ok);
	dood_canvas_fill_rect(cnv, 0, 0, ok->width, ok->height);

	dood_canvas_save_ppm(cnv, "out.ppm");

	dood_canvas_free(ok);
	dood_canvas_free(cnv);
	return 0;
}
