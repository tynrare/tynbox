#include "tests.h"
#include "math.h"

typedef struct LerpTestEntity {
  float threshold;
  float elapsed;
  float pos;
} LerpTestEntity;

LerpTestEntity lte0 = {10, 0, 0};
LerpTestEntity lte1 = {30, 0, 0};
LerpTestEntity lte2 = {100, 0, 0};

float dlerp(float a, float b, float decay, float dt) {
	return b + (a - b) * expf(-decay * dt);
}

static void _update_lte(LerpTestEntity *lte, float tpos) {
	double rdt = GetFrameTime();
	lte->elapsed += rdt * 1e3;
	if (lte->elapsed < lte->threshold) {
		return;
	}

	float dt = lte->elapsed * 1e-3;
	lte->elapsed = 0;

	float f = 0.1;

	// pow method
	//float fdt = dt / 0.03;
	//float df = 1-powf((1 - f), fdt);
	//lte->pos = Lerp(lte->pos, tpos, df);
	
	// exp method
	lte->pos = dlerp(lte->pos, tpos, 1, dt);
}

void _draw_test_lerp() {
	int tpos = GetMouseY();
	_update_lte(&lte0, tpos);
	_update_lte(&lte1, tpos);
	_update_lte(&lte2, tpos);

	DrawCircle(100, lte0.pos, 16, RED);
	DrawCircle(100, lte1.pos, 16, GREEN);
	DrawCircle(100, lte2.pos, 16, BLUE);
}
