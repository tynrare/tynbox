#include "include/editor0.h"
#include <math.h>
#include <raylib.h>
#include <raymath.h>

static void _dispose(Editor0State *state);
static STAGEFLAG _step(Editor0State *state, STAGEFLAG flags);
static void _draw(Editor0State *state);

#define RENDER_WIDTH 512.0f
#define RENDER_HEIGHT 512.0f
#define BOXES_AMOUNT 16

Editor0State *editor0_init(TynStage *stage) {
  Editor0State *state = MemAlloc(sizeof(Editor0State));

  state->edit_draw_mode = EDIT_DRAW_MODE_NONE;

  state->brush_boxes = MemAlloc(sizeof(BrushBox) * BOXES_AMOUNT);

  state->camera.zoom = 1;
  // state->camera.offset = (Vector2){RENDER_WIDTH / 2, RENDER_HEIGHT / 2};
  state->render_target = LoadRenderTexture(RENDER_WIDTH, RENDER_HEIGHT);

  state->camera3d.position = (Vector3){1, 1, 1};
  state->camera3d.target = (Vector3){0, 0, 0};
  state->camera3d.up = (Vector3){0, 1, 0};
  //state->camera3d.projection = CAMERA_ORTHOGRAPHIC;
  state->camera3d.projection = CAMERA_PERSPECTIVE;
  state->camera3d.fovy = 54;

  stage->state = state;
  stage->frame = (TynFrame){&_dispose, &_step, &_draw};

  return state;
}

static void _dispose(Editor0State *state) {
  UnloadRenderTexture(state->render_target);

  return;
}

static void _step_test_ray(Editor0State *state) {
  state->pointer_collision.hit = false;
  Ray ray = GetScreenToWorldRay(GetMousePosition(), state->camera3d);
  RayCollision plane_hit_info = {0};

  Vector3 g0 = (Vector3){-50.0f, 0.0f, -50.0f};
  Vector3 g1 = (Vector3){-50.0f, 0.0f, 50.0f};
  Vector3 g2 = (Vector3){50.0f, 0.0f, 50.0f};
  Vector3 g3 = (Vector3){50.0f, 0.0f, -50.0f};

  if (state->edit_draw_mode == EDIT_DRAW_MODE_NONE) {
    plane_hit_info = GetRayCollisionQuad(ray, g0, g1, g2, g3);
  } else if (state->edit_draw_mode == EDIT_DRAW_MODE_LEN) {
    // will use non-default plane
    plane_hit_info = GetRayCollisionQuad(ray, g0, g1, g2, g3);
  } else if (state->edit_draw_mode == EDIT_DRAW_MODE_HEIGHT) {
    Vector3 axis = {1, 0, 0};
    g0 = Vector3RotateByAxisAngle(g0, axis, 90 * DEG2RAD);
    g1 = Vector3RotateByAxisAngle(g1, axis, 90 * DEG2RAD);
    g2 = Vector3RotateByAxisAngle(g2, axis, 90 * DEG2RAD);
    g3 = Vector3RotateByAxisAngle(g3, axis, 90 * DEG2RAD);
		TraceLog(LOG_INFO, "%f,%f,%f", g0.x, g0.y, g0.z);
    g0 = Vector3Add(g0, state->edit_draw_points[1]);
    g1 = Vector3Add(g1, state->edit_draw_points[1]);
    g2 = Vector3Add(g2, state->edit_draw_points[1]);
    g3 = Vector3Add(g3, state->edit_draw_points[1]);
    plane_hit_info = GetRayCollisionQuad(ray, g0, g1, g2, g3);
  }

  if (plane_hit_info.hit) {
    state->pointer_collision = plane_hit_info;
  }
}

static BrushBox *getAvailableBoundingBox(Editor0State *state) {
  for (int i = 0; i < BOXES_AMOUNT; i++) {
    BrushBox *box = &state->brush_boxes[i];
    if (!box->active) {
      return box;
    }
  }

  return 0;
}

static void _edit_brush_confirm(Editor0State *state) {
  BrushBox *box = getAvailableBoundingBox(state);
  if (!box) {
    return;
  }
  box->active = true;
  const Vector3 *pp1 = &state->edit_draw_points[0];
  const Vector3 *pp2 = &state->edit_draw_points[1];
  const Vector3 *pp3 = &state->edit_draw_points[2];
  Vector3 min = {fminf(pp1->x, pp2->x), fminf(pp1->y, pp3->y),
                       fminf(pp1->z, pp2->z)};
  Vector3 max = {fmaxf(pp1->x, pp2->x), fmaxf(pp1->y, pp3->y),
                       fmaxf(pp1->z, pp2->z)};
  box->box.min = min;
  box->box.max = max;
}

static void _step_edit_brush(Editor0State *state) {
  if (!IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
    return;
  }

  if (!state->pointer_collision.hit) {
    return;
  }

  state->edit_draw_normal = state->pointer_collision.normal;

  if (state->edit_draw_mode == EDIT_DRAW_MODE_NONE) {
    state->edit_draw_points[0] = state->pointer_collision.point;
    state->edit_draw_mode = EDIT_DRAW_MODE_LEN;
  } else if (state->edit_draw_mode == EDIT_DRAW_MODE_LEN) {
    state->edit_draw_points[1] = state->pointer_collision.point;
    state->edit_draw_mode = EDIT_DRAW_MODE_HEIGHT;
  } else if (state->edit_draw_mode == EDIT_DRAW_MODE_HEIGHT) {
    state->edit_draw_mode = EDIT_DRAW_MODE_NONE;
    state->edit_draw_points[2] = state->pointer_collision.point;

    _edit_brush_confirm(state);
  }
}

static STAGEFLAG _step(Editor0State *state, STAGEFLAG flags) {
  state->elapsed += GetFrameTime();
  _step_test_ray(state);
  _step_edit_brush(state);

  return flags;
}

static void _draw2d(Editor0State *state) { return; }

static void _drawBrushBoxes(Editor0State *state) {
  for (int i = 0; i < BOXES_AMOUNT; i++) {
    BrushBox *box = &state->brush_boxes[i];
    if (!box->active) {
      continue;
    }

    DrawBoundingBox(box->box, BLUE);
  }
}

static void _draw3d(Editor0State *state) {
  DrawGrid(10, 0.1f);

  _drawBrushBoxes(state);

  if (state->pointer_collision.hit) {
    DrawCube(state->pointer_collision.point, 0.01, 0.01, 0.01, RED);
  }

  if (state->edit_draw_mode != EDIT_DRAW_MODE_NONE) {
    float h = 0;
    float w = 0;
    float l = 0;
    Vector3 center = {0};

    Vector3 p1 = state->edit_draw_points[0];
    Vector3 p2 = state->edit_draw_points[1];
    Vector3 p3 = state->pointer_collision.point;
    if (state->edit_draw_mode == EDIT_DRAW_MODE_LEN) {
      p2 = state->pointer_collision.point;
    } else if (state->edit_draw_mode == EDIT_DRAW_MODE_HEIGHT) {
    }
    w = fabs(p1.x - p2.x);
    l = fabs(p1.z - p2.z);
    h = fabs(p1.y - p3.y);
    center = Vector3Scale(Vector3Add(p1, p2), 0.5);
    center.y = (p3.y + p1.y) / 2;

    DrawCube(center, w, h, l, WHITE);
  }

  /*
float cubesize = 0.4f;
DrawCube((Vector3){0, sin(state->elapsed) * 0.2, 0}, cubesize * 0.5f,
     cubesize * 0.5f, cubesize * 0.5f, RAYWHITE);
DrawCubeWires((Vector3){0, sin(state->elapsed) * 0.2, 0}, cubesize, cubesize,
          cubesize, RAYWHITE);
                                                          */
}

static void _draw(Editor0State *state) {
  BeginTextureMode(state->render_target);
  ClearBackground(BLANK);

  BeginMode2D(state->camera);

  _draw2d(state);

  EndMode2D();

  BeginMode3D(state->camera3d);

  _draw3d(state);

  EndMode3D();

  EndTextureMode();

  ClearBackground(BLANK);

  float w = GetScreenWidth();
  float h = GetScreenHeight();
  float ratio = h / w;
  Rectangle source = (Rectangle){0, 0, RENDER_WIDTH, -RENDER_HEIGHT};
  Rectangle dest =
      (Rectangle){(w - w * ratio) / 2, 0, w * ratio, GetScreenHeight()};
  DrawTexturePro(state->render_target.texture, source, dest, (Vector2){0, 0}, 0,
                 WHITE);
  return;
}

// --- no found GetScriinToWorldRay source binds

// Get a ray trace from screen position (i.e mouse)
Ray GetScreenToWorldRay(Vector2 position, Camera camera) {
  return GetScreenToWorldRayEx(position, camera, (float)GetScreenWidth(),
                               (float)GetScreenHeight());
}

#define RL_CULL_DISTANCE_NEAR                                                  \
  0.01 // Default projection matrix near cull distance
#define RL_CULL_DISTANCE_FAR                                                   \
  1000.0 // Default projection matrix far cull distance
         //
// Get a ray trace from the screen position (i.e mouse) within a specific
// section of the screen
Ray GetScreenToWorldRayEx(Vector2 position, Camera camera, float width,
                          float height) {
  Ray ray = {0};

  // Calculate normalized device coordinates
  // NOTE: y value is negative
  float x = (2.0f * position.x) / width - 1.0f;
  float y = 1.0f - (2.0f * position.y) / height;
  float z = 1.0f;

  // Store values in a vector
  Vector3 deviceCoords = {x, y, z};

  // Calculate view matrix from camera look at
  Matrix matView = MatrixLookAt(camera.position, camera.target, camera.up);

  Matrix matProj = MatrixIdentity();

  if (camera.projection == CAMERA_PERSPECTIVE) {
    // Calculate projection matrix from perspective
    matProj = MatrixPerspective(camera.fovy * DEG2RAD,
                                ((double)width / (double)height),
                                RL_CULL_DISTANCE_NEAR, RL_CULL_DISTANCE_FAR);
  } else if (camera.projection == CAMERA_ORTHOGRAPHIC) {
    double aspect = (double)width / (double)height;
    double top = camera.fovy / 2.0;
    double right = top * aspect;

    // Calculate projection matrix from orthographic
    matProj = MatrixOrtho(-right, right, -top, top, 0.01, 1000.0);
  }

  // Unproject far/near points
  Vector3 nearPoint = Vector3Unproject(
      (Vector3){deviceCoords.x, deviceCoords.y, 0.0f}, matProj, matView);
  Vector3 farPoint = Vector3Unproject(
      (Vector3){deviceCoords.x, deviceCoords.y, 1.0f}, matProj, matView);

  // Unproject the mouse cursor in the near plane.
  // We need this as the source position because orthographic projects, compared
  // to perspective doesn't have a convergence point, meaning that the "eye" of
  // the camera is more like a plane than a point.
  Vector3 cameraPlanePointerPos = Vector3Unproject(
      (Vector3){deviceCoords.x, deviceCoords.y, -1.0f}, matProj, matView);

  // Calculate normalized direction vector
  Vector3 direction = Vector3Normalize(Vector3Subtract(farPoint, nearPoint));

  if (camera.projection == CAMERA_PERSPECTIVE)
    ray.position = camera.position;
  else if (camera.projection == CAMERA_ORTHOGRAPHIC)
    ray.position = cameraPlanePointerPos;

  // Apply calculated vectors to ray
  ray.direction = direction;

  return ray;
}
