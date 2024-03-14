#include "include/game_tynmaze.h"
#include "include/tyncommons.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

int TynmazeDispose(TynmazeState *state);
STAGEFLAG TynmazeStep(TynmazeState *state, STAGEFLAG flags);
int TynmazeDraw(TynmazeState *state);

float distlerp(float a, float b, float t) { 
    float tt = min(1.0f, 1.0f / (abs(b - a) + 0.01));
    return a + (b - a) * t * tt; 
}

void _TynmazeInitShader(TynmazeState* state) {
    Shader shader = LoadShader("res/shaders/lighting.vs", "res/shaders/fog.fs");
    shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(shader, "matModel");
    shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");

    int ambientLoc = GetShaderLocation(shader, "ambient");
    SetShaderValue(shader, ambientLoc, (float[4]) { 1.0f, 1.0f, 1.0f, 1.0f }, SHADER_UNIFORM_VEC4);

    float fogDensity = 0.15f;
    int fogDensityLoc = GetShaderLocation(shader, "fogDensity");
    SetShaderValue(shader, fogDensityLoc, &fogDensity, SHADER_UNIFORM_FLOAT);

    state->shader = shader;
}

void _TynmazeInit(TynmazeState *state) {

  // --- camera
  Camera3D camera = (Camera3D){0};
  camera.position = (Vector3){0.0f, 0.6f, 0.0f}; // Camera position
  camera.target = (Vector3){0.0f, 0.5f, 1.0f};   // Camera looking at point
  camera.up =
      (Vector3){0.0f, 1.0f, 0.0f}; // Camera up vector (rotation towards target)
  camera.fovy = 90.0f;             // Camera field-of-view Y
  camera.projection = CAMERA_PERSPECTIVE;

  state->camera = camera;
  state->viewMode = TMZ_VIEW_MODE_PAWN_FP;

  // --- textures
  state->tex_mark =
      LoadTexture("res/tynmaze/flag_triangle.png");

  // --- map
  Image imMap = LoadImage("res/tynmaze/maze-0.png"); // Load cubicmap image (RAM)
  state->cubicmap =
      LoadTextureFromImage(imMap); // Convert image to texture to display (VRAM)
  Mesh mesh = GenMeshCubicmap(imMap, (Vector3){1.0f, 1.0f, 1.0f});
  state->model = LoadModelFromMesh(mesh);
  _TynmazeInitShader(state);
  state->model.materials[0].shader = state->shader;

  // NOTE: By default each cube is mapped to one part of texture atlas
  state->texture =
      LoadTexture("res/tynmaze/cubicmap_atlas.png"); // Load map texture
  state->model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture =
      state->texture; // Set map diffuse texture

  // Get map image data to be used for collision detection
  state->mapPixels = LoadImageColors(imMap);
  UnloadImage(imMap); // Unload image from RAM

  // --- pawn
  state->pawn.mapPosition = (Vector3){-0.0f, 0.0f, -0.0f}; // Set model position
  state->pawn.inputDirection = (Vector2){0.0f, 0.0f};
  state->pawn.playerPosition = (Vector2){1.0f, 1.0f};
  state->pawn.playerTurn = 0.0f;
  state->pawn.cameraRot = 180.0f;
  state->pawn.steps = 0;
  state->pawn.tagsCount = 0;
  state->pawn.tagIndex = 0;
}

static void cmd(TynmazeState* state, char* command) {
    if (strcmp(command, "mode fp") == 0) {
        state->viewMode = TMZ_VIEW_MODE_PAWN_FP;
    } else if (strcmp(command, "mode topdown") == 0) {
        state->viewMode = TMZ_VIEW_MODE_PAWN_TOPDOWN;
    } else if (strcmp(command, "mode free") == 0) {
        state->viewMode = TMZ_VIEW_MODE_FREE;
    }
}

TynmazeState *TynmazeInit(TynStage *stage) {
  TynmazeState *state = malloc(sizeof(TynmazeState));

  _TynmazeInit(state);
  
  stage->state = state;
  stage->frame = (TynFrame){&TynmazeDispose, &TynmazeStep, &TynmazeDraw, NULL, &cmd };

  return stage->state;
}

int TynmazeDispose(TynmazeState *state) {
  UnloadImageColors(state->mapPixels); // Unload color array

  UnloadTexture(state->tex_mark);
  UnloadTexture(state->cubicmap); // Unload cubicmap texture
  UnloadTexture(state->texture);  // Unload map texture
  UnloadModel(state->model);      // Unload map model
  UnloadShader(state->shader);
  free(state);
}

void TynmazeStepPawn(TynmazeState* state) {
    state->pawn.inputDirection.x = 0.0f;
    state->pawn.inputDirection.y = 0.0f;

    if (IsKeyPressed(KEY_W)) {
        state->pawn.inputDirection.x = 1.0f;
    }
    else if (IsKeyPressed(KEY_S)) {
        state->pawn.inputDirection.x = -1.0f;
    }
    else if (IsKeyPressed(KEY_A)) {
        state->pawn.inputDirection.y = 1.0f;
    }
    else if (IsKeyPressed(KEY_D)) {
        state->pawn.inputDirection.y = -1.0f;
    }
    else if (IsKeyPressed(KEY_E)) {
        size_t length = sizeof(state->pawn.tagPositions) / sizeof(Vector2);
        int index = state->pawn.tagIndex++ % TYNMAZE_PAWN_TAGS_LIMIT;
        float x = state->pawn.playerPosition.x + sinf(state->pawn.playerTurn) * 0.25f;
        float y = state->pawn.playerPosition.y + cosf(state->pawn.playerTurn) * 0.25f;
        state->pawn.tagPositions[index] = (Vector2){ x, y };
        state->pawn.tagsCount = min(state->pawn.tagsCount + 1, TYNMAZE_PAWN_TAGS_LIMIT);
    }

    if (state->pawn.inputDirection.x) {
        state->pawn.steps += 1;
    }

    // rotate
    state->pawn.playerTurn += PI * 0.5f * state->pawn.inputDirection.y;

    // move
    int collider = 0;
    while (!collider && state->pawn.inputDirection.x) {
        float newx = roundf(state->pawn.playerPosition.x +
            sinf(state->pawn.playerTurn) * state->pawn.inputDirection.x);
        float newy = roundf(state->pawn.playerPosition.y +
            cosf(state->pawn.playerTurn) * state->pawn.inputDirection.x);
        collider =
            state->mapPixels[(int)(newy)*state->cubicmap.width + (int)(newx)].r;
        /*
        if(inputDirection.x != 0.0f) {
                printf("newx: %f, newy: %f, turn: %f \n", newx, newy, playerTurn);
        }
        */
        if (collider == 0) {
            state->pawn.playerPosition.x = newx;
            state->pawn.playerPosition.y = newy;
        }
        if (IsKeyDown(KEY_LEFT_SHIFT)) {
            break;
        }
    }

    state->pawn.cameraRot =
        rlerp(state->pawn.cameraRot, state->pawn.playerTurn, 0.5);

    switch (state->viewMode) {
    case TMZ_VIEW_MODE_PAWN_FP:
        state->camera.position.x =
            distlerp(state->camera.position.x, state->pawn.playerPosition.x, 0.5);
        state->camera.position.y = 0.5f;
        state->camera.position.z =
            distlerp(state->camera.position.z, state->pawn.playerPosition.y, 0.5);
        state->camera.target.x =
            state->camera.position.x + sinf(state->pawn.cameraRot);
        state->camera.target.y = state->camera.position.y;
        state->camera.target.z =
            state->camera.position.z + cosf(state->pawn.cameraRot);
        break;
    case TMZ_VIEW_MODE_PAWN_TOPDOWN: {
        float x = state->pawn.playerPosition.x - sinf(state->pawn.cameraRot);
        float y = state->pawn.playerPosition.y - cosf(state->pawn.cameraRot);
        state->camera.position.x = distlerp(state->camera.position.x, x, 0.5f);
        state->camera.position.y = 3.0f;
        state->camera.position.z = distlerp(state->camera.position.z, y, 0.5f);
        state->camera.target.x = distlerp(state->camera.target.x, state->pawn.playerPosition.x, 0.55f);
        state->camera.target.y = 0.5f;
        state->camera.target.z = distlerp(state->camera.target.z, state->pawn.playerPosition.y, 0.55f);
        } break;

    }

    SetShaderValue(state->shader, state->shader.locs[SHADER_LOC_VECTOR_VIEW], &state->camera.position.x, SHADER_UNIFORM_VEC3);

}

void TynmazeStepFree(TynmazeState* state) {
    state->camera.position.x = state->cubicmap.width / 2;
    state->camera.position.y = 40.0f;
    state->camera.position.z = state->cubicmap.width / 2;
    state->camera.target.x = state->camera.position.x + 0.01f;
    state->camera.target.y = state->camera.position.y - 1;
    state->camera.target.z = state->camera.position.z;
}

STAGEFLAG TynmazeStep(TynmazeState *state, STAGEFLAG flags) { 
    switch (state->viewMode) {
    case TMZ_VIEW_MODE_PAWN_FP:
    case TMZ_VIEW_MODE_PAWN_TOPDOWN:
        TynmazeStepPawn(state);
        break;
    case TMZ_VIEW_MODE_FREE:
        TynmazeStepFree(state);
    }
    return flags; 
}

int TynmazeDraw(TynmazeState *state) {
  BeginMode3D(state->camera);
  DrawModel(state->model, state->pawn.mapPosition, 1.0f, WHITE); // Draw maze map

  for (int i = 0; i < min(state->pawn.tagsCount, TYNMAZE_PAWN_TAGS_LIMIT); i++)
  {
      float x = state->pawn.tagPositions[i].x;
      float y = state->pawn.tagPositions[i].y;
      DrawBillboard(state->camera, state->tex_mark, (Vector3) { x, 0.5f, y }, 0.25f, WHITE);
      //DrawCube((Vector3) { x, 0.5f, y }, 0.25f, 0.25f, 0.25f, BLUE);
      /*
      DrawText(
          TextFormat(
              "#%i: %ix%i", i, (int)x, (int)y),
          10, 80 + i * 14, 12, BLACK);
          */
  }
  EndMode3D();

  DrawFPS(10, 10);
  DrawText(TextFormat("Steps: %i", state->pawn.steps), 10, 50, 10, BLACK);

  return 0;
}
