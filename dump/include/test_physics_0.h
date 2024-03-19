#include "tynroar_lib.h"
#include "raylib.h"
#include "box2d/box2d.h"

#ifndef TEST_PHYSICS0_H
#define TEST_PHYSICS0_H

typedef struct {
	b2WorldId worldId;
	b2BodyId groundBodyId;
	Camera2D camera;

	RenderTexture2D texdataset;
	Shader shader;
} TestPhysics0State;

TestPhysics0State* TestPhysics0Init(TynStage* stage);

#endif