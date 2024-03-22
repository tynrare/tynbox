#include "tynroar_lib.h"
#include "deferred_render.h"
#include "raylib.h"

#ifndef EDITOR0_H
#define EDITOR0_H

typedef struct {
	Matrix *transform;
	BoundingBox box;
	bool active;
} BrushBox;

typedef enum {
	EDIT_DRAW_MODE_NONE = 0,
	EDIT_DRAW_MODE_LEN = 1,
	EDIT_DRAW_MODE_HEIGHT = 2,
	__EDIT_DRAW_MODE_COUNT
} EditDrawMode;

typedef struct {
	Material material_base;
	Material material_second;
	Material material_tilefloor;
} Editor0Assets;

typedef struct {
	Camera2D camera;
	Camera3D camera3d;
	double elapsed;

	DeferredRenderState deferred_render;
	RenderTexture render_target;
	RayCollision pointer_collision;
	EditDrawMode edit_draw_mode;
	Vector3 edit_draw_normal;
	Vector3 edit_draw_initial_normal;
	Vector3 edit_draw_points[3];
	BrushBox *brush_boxes;
	Matrix *brush_boxes_transforms;
	Mesh cube;
	Mesh plane;
	Editor0Assets assets;
} Editor0State;

Editor0State *editor0_init(TynStage *stage);

#endif
