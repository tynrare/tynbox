#include "tynroar_lib.h"
#include <raylib.h>

#ifndef GAME_TYNMAZE_H
#define GAME_TYNMAZE_H

#define TYNMAZE_PAWN_TAGS_LIMIT 10

typedef enum {
	TMZ_VIEW_MODE_PAWN_FP,
	TMZ_VIEW_MODE_FREE,
	TMZ_VIEW_MODE_PAWN_TOPDOWN
} TynmazeViewMode;

typedef struct {
  Vector3 mapPosition; // Set model position
  Vector2 inputDirection;
  Vector2 playerPosition;
  float playerTurn;
  float cameraRot;
  int steps;
  int tagsCount;
  int tagIndex;
  Vector2 tagPositions[TYNMAZE_PAWN_TAGS_LIMIT];
} TynmazePawn;

typedef struct {
  Camera3D camera;
  TynmazePawn pawn;

  Color *mapPixels;
  Texture2D cubicmap;
  Texture2D texture;
  Texture2D tex_mark;
  Model model;
  Shader shader;
  TynmazeViewMode viewMode;
} TynmazeState;

TynmazeState *TynmazeInit(TynStage *stage);

#endif GAME_TYNMAZE_H