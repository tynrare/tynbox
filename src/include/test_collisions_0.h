#include "tynroar_lib.h"
#include "raylib.h"

#ifndef TEST_COLLISIONS0_H
#define TEST_COLLISIONS0_H

typedef struct {
	int testbox_posx;
	int testbox_posy;
} TestCollisions0State;

TestCollisions0State* TestCollisions0Init(TynStage* stage);

#endif