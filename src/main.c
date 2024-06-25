// @tynroar

#include "root.h"
#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

#include "./tests.h"

bool active = false;

static const int VIEWPORT_W = 0x320;
static const int VIEWPORT_H = 0x1c2;

static int viewport_w = VIEWPORT_W;
static int viewport_h = VIEWPORT_H;

typedef enum BOXMODES {
	BOXMODE_TEST_BARYCENTRIC = 0,
	BOXMODE_TEST_LERP = 1,
	__BOXMODES_COUNT__
} BOXMODE;

static BOXMODE boxmode = BOXMODE_TEST_BARYCENTRIC;

static void draw() {
  ClearBackground(BLACK);

	switch (boxmode) {
		case BOXMODE_TEST_BARYCENTRIC:
			_draw_test_barycentric();
			DrawText("tynbox: barycentric", 16, 16, 20, WHITE);
			break;
		case BOXMODE_TEST_LERP:
			_draw_test_lerp();
			DrawText("tynbox: lerp", 16, 16, 20, WHITE);
			break;
		default:
			DrawText("tynbox: ...", 16, 16, 20, WHITE);
			break;
	}

  DrawText("tynbox", viewport_w - 16 - 72, viewport_h - 16 - 20, 20, WHITE);
}

static void dispose() {}

static void init() { dispose(); }

static void inputs() {
	if (IsKeyPressed(KEY_SPACE)) {
		boxmode = (boxmode + 1) % __BOXMODES_COUNT__;
	}
}

static long resize_timestamp = -1;
static const float resize_threshold = 0.3;
static Vector2 requested_viewport = {VIEWPORT_W, VIEWPORT_H};
static void equilizer() {
  const int vw = GetScreenWidth();
  const int vh = GetScreenHeight();

  const long now = GetTime();

  // thresholds resizing
  if (requested_viewport.x != vw || requested_viewport.y != vh) {
    requested_viewport.x = vw;
    requested_viewport.y = vh;

    // first resize triggers intantly (important in web build)
    if (resize_timestamp > 0) {
      resize_timestamp = now;
      return;
    }
  }

  // reinits after riseze stops
  const bool resized =
      requested_viewport.x != viewport_w || requested_viewport.y != viewport_h;
  if (resized && now - resize_timestamp > resize_threshold) {
    resize_timestamp = now;
    viewport_w = vw;
    viewport_h = vh;
    // init();
  }
}

void step(void) {
  equilizer();

  inputs();

  BeginDrawing();
  draw();
  EndDrawing();
}

void loop() {
#if defined(PLATFORM_WEB)
  emscripten_set_main_loop(step, 0, 1);
#else

  while (!WindowShouldClose()) {
    step();
  }
#endif
}

int main(void) {
  const int seed = 2;

  InitWindow(viewport_w, viewport_h, "tynbox");
  SetWindowState(FLAG_WINDOW_RESIZABLE);
  SetTargetFPS(60);
  SetRandomSeed(seed);

  init();

  active = true;
  loop();

  dispose();
  CloseWindow();

  return 0;
}
