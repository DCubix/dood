#include <stdio.h>
#include <math.h>

#define DOOD_IMPLEMENTATION
#include "dood.h"

#define BLUR_KERNEL 9

Color blur(Canvas c, float x, float y) {
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
	Canvas cnv = dood_canvas_new(256, 256);
	dood_canvas_clear(cnv);
	
	Canvas ok = dood_canvas_load_ppm("ok.ppm");
	dood_canvas_draw_canvas(cnv, ok, 0, 0, 128, 128,  64, 64, 128, 128);
	dood_canvas_free(ok);

	dood_canvas_set_shader(cnv, &blur);
	for (int pass = 0; pass < 5; pass++)
		dood_canvas_fill_rect(cnv, 0, 0, 256, 256);

	dood_canvas_save_ppm(cnv, "out.ppm");

	dood_canvas_free(cnv);
	return 0;
}
