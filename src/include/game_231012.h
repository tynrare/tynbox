#include "raylib.h"
#include "raymath.h"
#include "tynroar_lib.h"

#define DEBUG 0

#ifndef GAME231012_H
#define GAME231012_H

typedef enum PAWN_CONTROL_MODE {
	PAWN_CONTROL_MODE_POINTER = 0,
	PAWN_CONTROL_MODE_WASD = 1
} PAWN_CONTROL_MODE;

typedef struct Sprite {
	Vector2 position;
	Vector2 anchor;
	Texture2D texture;
	float rotation;
	float scale;
} Sprite;

typedef struct G231012_PawnState {
	Vector2 position;
	Vector2 direction;
	Vector2 targetPosition;
	Vector2 lookAt;
	float speed;
	Vector2 lookDirection;
	PAWN_CONTROL_MODE control_mode;
	bool alive;
	double action_timestamp;
} G231012_PawnState;

typedef struct G231012_PawnConfig {
	float speed;
	float forceAcc;
	float forceBreak;
	float rotDump;
	float action_threshold;
} G231012_PawnConfig;

typedef struct G231012_BulletConfig {
	float speed;
	float lifetime;
} G231012_BulletConfig;

typedef struct G231012_BulletState {
	float timestamp;
	float speed;
	Vector2 position;
	Vector2 direction;
	bool alive;
} G231012_BulletState;

typedef struct G231012_GameAssets {
	Sprite crosshair;
	Sprite playership;
	Sprite tilefloor;
	Sprite locationmark;
	Sprite botship;
	Sprite bullet;
} G231012_GameAssets;

typedef struct G231012_GameState {
	G231012_GameAssets assets;
	G231012_PawnState pawn;
	G231012_PawnConfig pawnConfig;
	G231012_PawnConfig botConfig;
	G231012_BulletConfig bulletConfig;
	G231012_PawnState *bots;
	Sprite *bot_sprites;
	G231012_BulletState *bullets;
	Sprite *bullet_sprites;
} G231012_GameState;

G231012_GameState *G231012_Init(TynStage *stage);

Sprite SpriteLoad(const char* fileName);
Sprite SpriteCreate(Texture2D texture);
Sprite SpriteInit(Sprite *s, Texture2D texture);
void SpriteDraw(Sprite* sprite);

#endif
