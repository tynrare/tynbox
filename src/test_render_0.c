#include "include/test_render_0.h"
#include "include/deferred_render.h"
#include <raymath.h>
#include <stdlib.h>

#include "external/rlights.h"

#if defined(PLATFORM_WEB)
#define GLSL_VERSION 100
#else
#define GLSL_VERSION 330
#endif

void TestRender0Dispose(TestRender0State *state);
STAGEFLAG TestRender0Step(TestRender0State *state, STAGEFLAG flags);
void TestRender0Draw(TestRender0State *state);

#define TEST_DEFERRED 1

void _TestRender0Init(TestRender0State *state) {
  // Define the camera to look into our 3d world
  Camera camera = {0};
  camera.position = (Vector3){4.0f, 4.0f, 4.0f}; // Camera position
  camera.target = (Vector3){0.0f, 1.0f, -1.0f};  // Camera looking at point
  camera.up =
      (Vector3){0.0f, 1.0f, 0.0f}; // Camera up vector (rotation towards target)
  camera.fovy = 45.0f;             // Camera field-of-view Y
  camera.projection = CAMERA_PERSPECTIVE; // Camera projection type

  state->model = LoadModel("res/test/d231117.obj");       // Load OBJ model
  state->texture = LoadTexture("res/test/palette_0.png"); // Load model texture
  state->tex_target = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
  // Load shader for model
  // NOTE: Defining 0 (NULL) for vertex shader forces usage of internal default
  // vertex shader
  state->shader =
      LoadShader(0, TextFormat("res/shaders/sobel-%i.fs", GLSL_VERSION));

  LoadDeferredRender(&state->deferred_render, GetScreenWidth(),
                     GetScreenHeight());
  CreateLight(LIGHT_DIRECTIONAL, (Vector3){-2, 1, -2}, Vector3Zero(), WHITE,
              state->deferred_render.deferredShader);

  // state->model.materials[0].shader = state->shader;
  // Set shader effect to 3d model
  state->model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = state->texture;
  state->model.materials[0].shader = state->deferred_render.gbufferShader;

  state->camera = camera;
}

TestRender0State *TestRender0Init(TynStage *stage) {
  TestRender0State *state = malloc(sizeof(TestRender0State));
  _TestRender0Init(state);

  stage->state = state;
  stage->frame =
      (TynFrame){&TestRender0Dispose, &TestRender0Step, &TestRender0Draw};

  return stage->state;
}

void TestRender0Dispose(TestRender0State *state) {
  UnloadShader(state->shader);   // Unload shader
  UnloadTexture(state->texture); // Unload texture
  UnloadModel(state->model);     // Unload model
  UnloadRenderTexture(state->tex_target);
  UnloadDeferredRender(&state->deferred_render);
  free(state);
}

STAGEFLAG TestRender0Step(TestRender0State *state, STAGEFLAG flags) {
  UpdateCamera(&state->camera, CAMERA_FIRST_PERSON);
  return flags;
}

void TestRender0Draw(TestRender0State *state) {
  Vector3 position = {0.0f, 0.0f, 0.0f};

#if TEST_DEFERRED
  BeginDrawDeferredRender(&state->deferred_render, state->camera);
  DrawModel(state->model, position, 0.1f, WHITE);
  EndDrawDeferredRender(&state->deferred_render, state->camera,
                        GetScreenWidth(), GetScreenHeight(), DEFERRED_SHADING);
  BeginMode3D(state->camera);                     // Begin 3d mode drawing
  DrawGrid(10, 1.0f);                             // Draw a grid
	EndMode3D();
#else
  BeginTextureMode(state->tex_target); // Enable drawing to texture
  ClearBackground(RAYWHITE);           // Clear texture background

  BeginMode3D(state->camera);                     // Begin 3d mode drawing
  DrawModel(state->model, position, 0.1f, WHITE); // Draw 3d model with texture
  DrawGrid(10, 1.0f);                             // Draw a grid
  EndMode3D();      // End 3d mode drawing, returns to orthographic 2d mode
  EndTextureMode(); // End drawing to texture (now we have a texture available
                    // for next passes)

  BeginShaderMode(state->shader);
  // NOTE: Render texture must be y-flipped due to default OpenGL coordinates
  // (left-bottom)
  DrawTextureRec(state->tex_target.texture,
                 (Rectangle){0, 0, (float)state->tex_target.texture.width,
                             (float)-state->tex_target.texture.height},
                 (Vector2){0, 0}, WHITE);
  EndShaderMode();

  DrawFPS(10, 10);
#endif
}
