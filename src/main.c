#include <stdio.h>
#include <math.h>

#define DOOD_IMPLEMENTATION
#include "dood.h"

int main(int argc, char** argv) {
	Canvas* cnv = dood_canvas_new(256, 256);

	for (int y = 0; y < 256; y++) {
		for (int x = 0; x < 256; x++) {
			float fx = (float)x / 256.0f;
			float fy = (float)y / 256.0f;

			dood_canvas_set_blend(cnv, BM_OPAQUE);
			dood_canvas_set_draw_color(cnv, fx, fy, 1.0f - (fx * fy), 1.0f);
			dood_canvas_draw_point(cnv, x, y);

			dood_canvas_set_blend(cnv, BM_ALPHA_BLEND);
			dood_canvas_set_draw_color(cnv, 1.0f, 0.0f, 0.0f, (sinf(((fx + fy) * 0.5f) * 16.0f) * 0.5f + 0.5f));
			dood_canvas_draw_point(cnv, x, y);
		}
	}

	dood_canvas_save_ppm(cnv, "out.ppm");

	dood_canvas_free(cnv);
	return 0;
}
