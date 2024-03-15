#include "tynroar_lib.h"
#include "collisions.h"
#include "raylib.h"

#ifndef GAME_PLATFORMER0_H
#define GAME_PLATFORMER0_H

typedef struct {
	bool 										active;
	b2AABB 									*aabb; 
} GamePlatformerCollider;

typedef struct {
	bool 										active;
	Vector2 								position;
	Vector2 								velocity;
	float 									mass;
	float 									bounce;
	float 									drag;
	float 									friction;
	b2AABB 								 *aabb;
} GamePlatformerBody;

typedef struct {
	GamePlatformerBody 			body;
	b2AABB 									aabb;
} GamePlatformerPawn;

typedef struct {
	GamePlatformerPawn 			pawn;
	GamePlatformerBody 		 *bodies;
	GamePlatformerCollider *colliders;
	b2AABB 								 *aabbs;
} GamePlatformer0State;

GamePlatformer0State* GamePlatformer0Init(TynStage* stage);

#endif

