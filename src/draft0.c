#include "include/draft0.h"
#include <math.h>
#include <raylib.h>

static void _dispose(Draft0State *state);
static STAGEFLAG _step(Draft0State *state, STAGEFLAG flags);
static void _draw(Draft0State *state);

#define RENDER_WIDTH 128.0f
#define RENDER_HEIGHT 128.0f

Draft0State *draft0_init(TynStage *stage) {
  Draft0State *state = MemAlloc(sizeof(Draft0State));

  state->camera.zoom = 1;
  // state->camera.offset = (Vector2){RENDER_WIDTH / 2, RENDER_HEIGHT / 2};
  state->render_target = LoadRenderTexture(RENDER_WIDTH, RENDER_HEIGHT);

  state->camera3d.position = (Vector3){1, 1, 1};
  state->camera3d.target = (Vector3){0, 0, 0};
  state->camera3d.up = (Vector3){0, 1, 0};
  state->camera3d.projection = CAMERA_ORTHOGRAPHIC;
  state->camera3d.fovy = 1;

  stage->state = state;
  stage->frame = (TynFrame){&_dispose, &_step, &_draw};

  return state;
}

static void _dispose(Draft0State *state) { return; }
static STAGEFLAG _step(Draft0State *state, STAGEFLAG flags) {
  state->elapsed += GetFrameTime();
  state->camera3d.position.x = sin(state->elapsed);
  state->camera3d.position.z = cos(state->elapsed);

  return flags;
}
static void _draw(Draft0State *state) {
  BeginTextureMode(state->render_target);
  ClearBackground(BLANK);

  BeginMode2D(state->camera);

  // DrawRectangle(0, 0, 16, 16, RED);
	int rec_pad = 4;
  Rectangle rec = (Rectangle){rec_pad, rec_pad,
                              RENDER_WIDTH - rec_pad * 2, RENDER_HEIGHT - rec_pad * 2};
  DrawRectangleRounded(rec, 0.2, 4, BLUE);

  EndMode2D();

  BeginMode3D(state->camera3d);

  float cubesize = 0.4f;
  DrawCube((Vector3){0, sin(state->elapsed) * 0.2, 0}, cubesize * 0.5f,
           cubesize * 0.5f, cubesize * 0.5f, RAYWHITE);
  DrawCubeWires((Vector3){0, sin(state->elapsed) * 0.2, 0}, cubesize, cubesize,
                cubesize, RAYWHITE);

  EndMode3D();

  EndTextureMode();

  ClearBackground(BLANK);

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
