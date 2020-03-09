#include <stdio.h>
#include <math.h>

#define DOOD_IMPLEMENTATION
#include "dood.h"

int main(int argc, char** argv) {
	Canvas prev = dood_canvas_load_ppm("out.ppm");
	Canvas cnv = dood_canvas_new(256, 256);

	dood_canvas_clear(cnv);

	dood_canvas_draw_canvas(cnv, prev, 0, 0, 256, 256, 0, 0, 128, 256);

	dood_canvas_save_ppm(cnv, "out2.ppm");

	dood_canvas_free(cnv);
	dood_canvas_free(prev);
	return 0;
}
