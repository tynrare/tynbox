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

LerpTestEntity slte0 = {10, 0, 0};
LerpTestEntity slte1 = {30, 0, 0};
LerpTestEntity slte2 = {100, 0, 0};

float dlerp(float a, float b, float decay, float dt) {
	return b + (a - b) * expf(-decay * dt);
}

float lerp(float a, float b, float t) { return a + (b - a) * t; }

static void _update_lte(LerpTestEntity *lte, float tpos) {
	double rdt = GetFrameTime();
	lte->elapsed += rdt * 1e3;
	if (lte->elapsed < lte->threshold) {
		return;
	}

	float dt = lte->elapsed * 1e-3;
	lte->elapsed = 0;


	// pow method
	//float f = 0.1;
	//float fdt = dt / 0.03;
	//float df = 1-powf((1 - f), fdt);
	//lte->pos = Lerp(lte->pos, tpos, df);
	
	// exp method
	lte->pos = dlerp(lte->pos, tpos, 1, dt);
}

static void _update_slte(LerpTestEntity *lte, float tpos) {
	double rdt = GetFrameTime();
	lte->elapsed += rdt * 1e3;
	if (lte->elapsed < lte->threshold) {
		return;
	}

	float dt = lte->elapsed * 1e-3;
	lte->elapsed = 0;

	float f = 0.1;

	lte->pos = lerp(lte->pos, tpos, f);
}

void _draw_test_lerp() {
	int tpos = GetMouseY();
	_update_lte(&lte0, tpos);
	_update_lte(&lte1, tpos);
	_update_lte(&lte2, tpos);

	DrawCircle(100, lte0.pos, 10, RED);
	DrawCircle(110, lte1.pos, 10, GREEN);
	DrawCircle(120, lte2.pos, 10, BLUE);
	DrawText("decay lerp", 100, 40, 20, WHITE);
	DrawText("b + (a - b) * exp(-decay * dt)", 100, 60, 20, WHITE);

	_update_slte(&slte0, tpos);
	_update_slte(&slte1, tpos);
	_update_slte(&slte2, tpos);

	DrawCircle(500, slte0.pos, 10, RED);
	DrawCircle(510, slte1.pos, 10, GREEN);
	DrawCircle(520, slte2.pos, 10, BLUE);
	DrawText("lerp", 500, 40, 20, WHITE);
	DrawText("a + (b - a) * t", 500, 60, 20, WHITE);

}
