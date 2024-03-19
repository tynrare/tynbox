#include "tynroar_lib.h"
#include "raylib.h"

#ifndef DRAFT0_H
#define DRAFT0_H

typedef struct {
	Camera2D camera;
	Camera3D camera3d;
	double elapsed;
	RenderTexture render_target;
} Draft0State;

Draft0State *draft0_init(TynStage *stage);

#endif
