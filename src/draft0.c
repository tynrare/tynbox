#include "include/draft0.h"
#include <raylib.h>
#include <math.h>

static void _dispose(Draft0State *state);
static STAGEFLAG _step(Draft0State *state, STAGEFLAG flags);
static void _draw(Draft0State *state);

#define RENDER_WIDTH 64.0f
#define RENDER_HEIGHT 64.0f

Draft0State *draft0_init(TynStage *stage) {
  Draft0State *state = MemAlloc(sizeof(Draft0State));

  state->camera.zoom = 1;
  state->camera.offset = (Vector2){RENDER_WIDTH / 2, RENDER_HEIGHT / 2};
  state->render_target = LoadRenderTexture(RENDER_WIDTH, RENDER_HEIGHT);

  stage->state = state;
  stage->frame = (TynFrame){&_dispose, &_step, &_draw};

  return state;
}

static void _dispose(Draft0State *state) { return; }
static STAGEFLAG _step(Draft0State *state, STAGEFLAG flags) { 
	Vector2 center = {16, 16};
  state->camera.rotation = sinf(GetTime()) * RAD2DEG;
  state->camera.target = center;
  state->camera.target.y += cosf(GetTime()) * 8;
	
	return flags; }
static void _draw(Draft0State *state) {
	Vector2 center = {16, 16};
  BeginTextureMode(state->render_target);
  ClearBackground(RAYWHITE);

  BeginMode2D(state->camera);

  // DrawRectangle(0, 0, 16, 16, RED);
  Rectangle rec = (Rectangle){0, 0, center.x * 2, center.y * 2};
  DrawRectangleRoundedLines(rec, 0.4, 4, 4, RED);

  EndMode2D();
  EndTextureMode();

  ClearBackground(RAYWHITE);

  float w = GetScreenWidth();
  float h = GetScreenHeight();
  float ratio = h / w;
  Rectangle source = (Rectangle){0, 0, RENDER_WIDTH, -RENDER_HEIGHT};
  Rectangle dest =
      (Rectangle){(w - w * ratio) / 2, 0, w * ratio, GetScreenHeight()};
  DrawTexturePro(state->render_target.texture, source, dest, (Vector2){0, 0}, 0,
                 WHITE);
  return;
}
