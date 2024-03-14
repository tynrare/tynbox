#include "include/game_231012.h"
#include <stdlib.h>

#ifndef V2CENTER
#define V2CENTER                                                               \
  (Vector2) { 0.5, 0.5 }
#endif

#ifndef V2UP
#define V2UP                                                                   \
  (Vector2) { 0, 1 }
#endif

float AngleDifference(float angle1, float angle2) {
  double diff = ((int)angle2 - (int)angle1 + 180) % 360 - 180;
  return diff < -180 ? diff + 360 : diff;
}

static void PawnWASDControls(struct G231012_PawnState *state,
                             struct G231012_PawnConfig *config);
static void PawnPointerControls(struct G231012_PawnState *state,
                                struct G231012_PawnConfig *config);
static void G231012Draw(G231012_GameState *state);
static int G231012Step(G231012_GameState *state, int flags);
static int G231012Dispose(G231012_GameState *state);

G231012_GameAssets load() {
  G231012_GameAssets ga = {
      SpriteLoad("res/crosshair.png"), SpriteLoad("res/playership.png"),
      SpriteLoad("res/tilefloor.png"), SpriteLoad("res/locationmark.png"),
      SpriteLoad("res/ship_C.png"),
  };

  return ga;
}

const short unsigned int BOTS_COUNT = 5;

G231012_GameState *G231012_Init(TynStage *stage) {
  G231012_PawnState pawnState = {
      (Vector2){256, 256}, V2UP, (Vector2){256, 256}, V2UP, 0, V2UP, 0, true};
  G231012_PawnConfig pawnConfig = {7.0f, 0.05f, 0.3f, 0.1f};
  G231012_PawnConfig botConfig = {4.0f, 0.15f, 0.2f, 0.2f};

  G231012_GameState *state = malloc(sizeof(G231012_GameState));
  state->assets = load();
  state->pawn = pawnState;
  state->pawnConfig = pawnConfig;
  state->botConfig = botConfig;

  state->bots =
      (G231012_PawnState *)MemAlloc(sizeof(G231012_PawnState) * BOTS_COUNT);
  state->bot_sprites = (Sprite *)MemAlloc(sizeof(Sprite) * BOTS_COUNT);

  for (int i = 0; i < BOTS_COUNT; i++) {
    G231012_PawnState *bot = &state->bots[i];
    Sprite *sprite = &state->bot_sprites[i];
    bot->control_mode = PAWN_CONTROL_MODE_POINTER;
    bot->alive = false;

    sprite->position = (Vector2){0.5f, 0.5f};
    sprite->anchor = (Vector2){0.5f, 0.5f};
    sprite->texture = state->assets.botship.texture;
    sprite->rotation = 0.0f;
    sprite->scale = 1.0f;
  }

  stage->state = state;
  stage->frame = (TynFrame){&G231012Dispose, &G231012Step, &G231012Draw};
  stage->flags = 0;

  return stage->state;
}

static int G231012Dispose(G231012_GameState *state) {
  UnloadTexture(state->assets.crosshair.texture);
  UnloadTexture(state->assets.playership.texture);
  UnloadTexture(state->assets.tilefloor.texture);
  UnloadTexture(state->assets.locationmark.texture);
  UnloadTexture(state->assets.botship.texture);
  MemFree(state->bots);

  return 0;
}

static void StepPawn(G231012_PawnState *pawn, G231012_PawnConfig *config,
                     Sprite *sprite) {
  pawn->lookDirection = Vector2Normalize(Vector2Lerp(
      pawn->lookDirection,
      Vector2Normalize(Vector2Subtract(pawn->lookAt, pawn->position)),
      config->rotDump));

  sprite->position = pawn->position;
  sprite->rotation = Vector2Angle(V2UP, pawn->lookDirection) * RAD2DEG - 180;
}

static void SpawnBots(G231012_GameState *state) {
  for (int i = 0; i < BOTS_COUNT; i++) {
    G231012_PawnState *bot = &state->bots[i];
    if (bot->alive) {
      continue;
    }

    bot->position =
				Vector2Add(state->pawn.position,
        Vector2Scale(Vector2Normalize((Vector2){GetRandomValue(-256, 256),
                                                GetRandomValue(-256, 256)}),
                     512)
				);
    bot->direction = V2UP;
    bot->targetPosition = (Vector2){256, 256};
    bot->speed = 0;
    bot->lookDirection = (Vector2){0, 0};
    bot->alive = true;
  }
}

static void StepBots(G231012_GameState *state) {
  SpawnBots(state);

  for (int i = 0; i < BOTS_COUNT; i++) {
    G231012_PawnState *bot = &state->bots[i];
    if (!bot->alive) {
      continue;
    }

    bot->targetPosition = state->pawn.position;
    bot->lookAt = state->pawn.position;
    PawnPointerControls(bot, &state->botConfig);
    StepPawn(bot, &state->pawnConfig, &state->bot_sprites[i]);
  }
}

static int G231012Step(G231012_GameState *state, int flags) {
  // pre
  Vector2 mousepos = GetMousePosition();

  if (state->pawn.control_mode == PAWN_CONTROL_MODE_POINTER &&
      GetKeyPressed()) {
    state->pawn.control_mode = PAWN_CONTROL_MODE_WASD;
  } else if (state->pawn.control_mode == PAWN_CONTROL_MODE_WASD &&
             IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
    state->pawn.control_mode = PAWN_CONTROL_MODE_POINTER;
  }

  if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
    state->pawn.targetPosition = mousepos;
  }

  state->pawn.lookAt = mousepos;

  switch (state->pawn.control_mode) {
  case PAWN_CONTROL_MODE_WASD:
    PawnWASDControls(&state->pawn, &state->pawnConfig);
    break;
  case PAWN_CONTROL_MODE_POINTER:
    PawnPointerControls(&state->pawn, &state->pawnConfig);
    break;
  }

  StepPawn(&state->pawn, &state->pawnConfig, &state->assets.playership);

  state->assets.crosshair.position = mousepos;
  state->assets.locationmark.position = state->pawn.targetPosition;
  state->assets.locationmark.rotation += 1;
  state->assets.locationmark.scale = 1.1 + sinf(GetTime()) * 0.1;

  StepBots(state);

  return flags;
}

static void DrawBots(G231012_GameState *state) {
  for (int i = 0; i < BOTS_COUNT; i++) {
    G231012_PawnState *bot = &state->bots[i];
    Sprite *sprite = &state->bot_sprites[i];
    if (!bot->alive) {
      continue;
    }
    SpriteDraw(sprite);
  }
}

static void G231012Draw(G231012_GameState *state) {
  DrawTexturePro(state->assets.tilefloor.texture, (Rectangle){0, 0, 1024, 1024},
                 (Rectangle){0, 0, 1024, 1024}, (Vector2){0, 0}, 0, WHITE);
  DrawText("TAB to open console.\n? to display commands", 16, 200, 20,
           LIGHTGRAY);

  SpriteDraw(&state->assets.crosshair);
  SpriteDraw(&state->assets.playership);
  SpriteDraw(&state->assets.locationmark);
  DrawBots(state);

#if DEBUG
  DrawLine(state->pawn.position.x, state->pawn.position.y,
           state->pawn.position.x + state->pawn.lookDirection.x * 100,
           state->pawn.position.y + state->pawn.lookDirection.y * 100, WHITE);
#endif
}

static void PawnPointerControls(struct G231012_PawnState *state,
                                struct G231012_PawnConfig *config) {
  Vector2 target = Vector2Subtract(
      state->targetPosition, Vector2Add(state->position, state->direction));
  float distance = Vector2Length(target);
  Vector2 direction = Vector2Normalize(target);
  Vector2 maxspeed = Vector2Scale(direction, fminf(distance, config->speed));

  Vector2 acceleration = Vector2Subtract(maxspeed, state->direction);
  Vector2 breakforce = Vector2Scale(
      state->direction,
      fminf(0,
            Vector2DotProduct(Vector2Normalize(state->direction), direction)));

  state->direction = Vector2Add(state->direction,
                                Vector2Scale(acceleration, config->forceAcc));
  state->direction = Vector2Add(state->direction,
                                Vector2Scale(breakforce, config->forceBreak));

  state->position.x += state->direction.x;
  state->position.y += state->direction.y;
}

static void PawnWASDControls(struct G231012_PawnState *state,
                             struct G231012_PawnConfig *config) {
  Vector2 inputDirection;

  inputDirection.x = 0;
  inputDirection.y = 0;

  // inputs
  if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))
    inputDirection.x += 1;
  if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))
    inputDirection.x -= 1;
  if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))
    inputDirection.y -= 1;
  if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))
    inputDirection.y += 1;

  float speed = (inputDirection.x || inputDirection.y || 0) * config->speed;
  state->speed = Lerp(state->speed, speed, config->forceAcc);

  Vector2Normalize(inputDirection);
  state->direction.x =
      Lerp(state->direction.x, inputDirection.x, config->rotDump);
  state->direction.y =
      Lerp(state->direction.y, inputDirection.y, config->rotDump);

  // apply move
  Vector2 move = Vector2Scale(state->direction, state->speed);
  state->position.x += move.x;
  state->position.y += move.y;
}

// ---

Sprite SpriteLoad(const char *fileName) {
  return SpriteCreate(LoadTexture(fileName));
}

Sprite SpriteCreate(Texture2D texture) {
  Sprite s = {(Vector2){0.0, 0.0}, (Vector2){0.5, 0.5}, texture, 0.0, 1.0};

  return s;
}

void SpriteDraw(Sprite *sprite) {
  const float x = sprite->texture.width * sprite->scale * sprite->anchor.x;
  const float y = sprite->texture.height * sprite->scale * sprite->anchor.y;
  const Vector2 v0 = Vector2Rotate((Vector2){x, y}, sprite->rotation * DEG2RAD);
  const Vector2 v1 =
      (Vector2){sprite->position.x - v0.x, sprite->position.y - v0.y};

  DrawTextureEx(sprite->texture, v1, sprite->rotation, sprite->scale, WHITE);
}
