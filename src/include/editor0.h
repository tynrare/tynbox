#include "tynroar_lib.h"
#include "raylib.h"

#ifndef EDITOR0_H
#define EDITOR0_H

typedef enum {
	EDIT_DRAW_MODE_NONE = 0,
	EDIT_DRAW_MODE_LEN = 1,
	EDIT_DRAW_MODE_HEIGHT = 2,
} EditDrawMode;

typedef struct {
	Camera2D camera;
	Camera3D camera3d;
	double elapsed;
	RenderTexture render_target;
	RayCollision pointer_collision;
	EditDrawMode edit_draw_mode;
	Vector3 edit_draw_normal;
	Vector3 edit_draw_points[3];
} Editor0State;

Editor0State *editor0_init(TynStage *stage);

#endif
