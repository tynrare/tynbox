#include <stdlib.h>

#include "include/test_shader_0.h"
#include "include/shader_sdf2d.h"
#include "rlgl.h"

#include <math.h>
#include <time.h> // Required for: localtime(), asctime()

#if defined(PLATFORM_WEB)
#define GLSL_VERSION 100
#else 
#define GLSL_VERSION 330
#endif


void TestShader0Dispose(TestShader0State *state);
STAGEFLAG TestShader0Step(TestShader0State *state, STAGEFLAG flags);
void TestShader0Draw(TestShader0State *state);

void _TestShader0Init(TestShader0State *state) {
  Raymarch2dInit(&state->texdataset, &state->shader);
  
  state->shaderAutoReloading = false;
  state->totalTime = 0;
  state->fragShaderFileModTime = 
		GetFileModTime(TextFormat(Raymarch2dfragShaderFileName, GLSL_VERSION));
}

TestShader0State* TestShader0Init(TynStage *stage) {
  TestShader0State *state = malloc(sizeof(TestShader0State));
  _TestShader0Init(state);
    
  stage->state = state;
  stage->frame =
      (TynFrame){&TestShader0Dispose, &TestShader0Step, &TestShader0Draw};

  return stage->state;
}

STAGEFLAG TestShader0Step(TestShader0State *state, STAGEFLAG flags) {
  Vector2 mouse = GetMousePosition();

  Raymarch2dBeginWrite(&state->texdataset, true);
  int shift = 0;
  shift = Raymarch2dWriteEntity(shift, CIRCLE, mouse, (Vector2){20, 0}, 0);
  shift = Raymarch2dWriteEntity(shift, BOX, (Vector2) { 100, 100 }, (Vector2) { 20, 20 }, -((int)(GetTime() * 100)) % 360);
  shift = Raymarch2dWriteEntity(shift, BOX, (Vector2){100, 150}, (Vector2){50, 20}, ((int)(GetTime() * 100)) % 360);
  Raymarch2dEndWrite();

  // blahblah
  // ===
  // Update
  //----------------------------------------------------------------------------------
  state->totalTime += GetFrameTime();
  float mousePos[2] = {mouse.x, mouse.y};

  // Set shader required uniform values
  int timeLoc = GetShaderLocation(state->shader, "time");
  int mouseLoc = GetShaderLocation(state->shader, "mouse");
  SetShaderValue(state->shader, timeLoc, &state->totalTime,
                 SHADER_UNIFORM_FLOAT);
  SetShaderValue(state->shader, mouseLoc, mousePos, SHADER_UNIFORM_VEC2);

  // Hot shader reloading
  if (state->shaderAutoReloading || (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))) {
    long currentFragShaderModTime =
        GetFileModTime(TextFormat(Raymarch2dfragShaderFileName, GLSL_VERSION));

    // Check if shader file has been modified
    if (currentFragShaderModTime != state->fragShaderFileModTime) {
      // Try reloading updated shader
      Shader updatedShader =
          LoadShader(0, TextFormat(Raymarch2dfragShaderFileName, GLSL_VERSION));

      if (updatedShader.id != rlGetShaderIdDefault()) // It was correctly loaded
      {
        UnloadShader(state->shader);
        state->shader = updatedShader;

        // Get shader locations for required uniforms
        int resolutionLoc = GetShaderLocation(state->shader, "resolution");

        // Reset required uniforms
        float resolution[2] = {(float)GetScreenWidth(),
                               (float)GetScreenHeight()};
        SetShaderValue(state->shader, resolutionLoc, resolution,
                       SHADER_UNIFORM_VEC2);
      }

      state->fragShaderFileModTime = currentFragShaderModTime;
    }
  }

  if (IsKeyPressed(KEY_A))
    state->shaderAutoReloading = !state->shaderAutoReloading;

  return flags;
}

void TestShader0Draw(TestShader0State *state) {
  int resolutionLoc = GetShaderLocation(state->shader, "resolution");
  int mouseLoc = GetShaderLocation(state->shader, "mouse");
  int timeLoc = GetShaderLocation(state->shader, "time");
  int datasetLoc = GetShaderLocation(state->shader, "dataset");

  Raymarch2dDraw(&state->texdataset, &state->shader);

  DrawText(TextFormat("PRESS [A] to TOGGLE SHADER AUTOLOADING: %s",
                      state->shaderAutoReloading ? "AUTO" : "MANUAL"),
           10, 10, 10, state->shaderAutoReloading ? RED : BLACK);

  Vector2 mouse = GetMousePosition();
  DrawCircleLines(mouse.x, mouse.y, 20, WHITE);

  /*
  DrawText(TextFormat("Shader last modification: %s",
                      asctime(localtime(&state->fragShaderFileModTime))),
           10, 430, 10, BLACK);
           */
}

void TestShader0Dispose(TestShader0State *state) {
  UnloadShader(state->shader);
  free(state);
}
