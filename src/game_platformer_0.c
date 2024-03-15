#include "include/game_platformer_0.h"
#include <raymath.h>
#include <stddef.h>
#include <stdlib.h>

static void _dispose(GamePlatformer0State *state);
static STAGEFLAG _step(GamePlatformer0State *state, STAGEFLAG flags);
static void _draw(GamePlatformer0State *state);

#define COLLIDERS_COUNT 16

static void init_body(GamePlatformerBody *body, b2AABB *aabb) {
  body->position = (Vector2){0, 0};
  body->velocity = (Vector2){0, 0};
  body->mass = 1;
  body->bounce = 0;
  body->friction = 0.1;
  body->drag = 0.003;
  body->active = false;
  body->aabb = aabb;
}

static void spawn_collider(GamePlatformer0State *state, int x, int y, int w,
                           int h) {
  for (int i = 0; i < COLLIDERS_COUNT; i++) {
    GamePlatformerCollider *collider = &state->colliders[i];
    if (collider->active) {
      continue;
    }

    b2AABB_Set(collider->aabb, x, y, w, h);
    collider->active = true;
    return;
  }
}

GamePlatformer0State *GamePlatformer0Init(TynStage *stage) {
  GamePlatformer0State *state = MemAlloc(sizeof(GamePlatformer0State));
  if (state == NULL) {
    return NULL;
  }

  state->colliders = MemAlloc(sizeof(GamePlatformerCollider) * COLLIDERS_COUNT);
  state->aabbs = MemAlloc(sizeof(b2AABB) * COLLIDERS_COUNT);

  for (int i = 0; i < COLLIDERS_COUNT; i++) {
    GamePlatformerCollider *collider = &state->colliders[i];
    b2AABB *aabb = &state->aabbs[i];
    collider->active = false;
    collider->aabb = aabb;
  }

  init_body(&state->pawn.body, &state->pawn.aabb);
  state->pawn.body.position = (Vector2){256, 256};
  b2AABB_Set(&state->pawn.aabb, 256, 256, 32, 32);

  spawn_collider(state, 256, 0, 512, 16);
  spawn_collider(state, 256, 512, 512, 16);
  spawn_collider(state, 0, 256, 16, 512);
  spawn_collider(state, 512, 256, 16, 512);

  stage->state = state;
  stage->frame = (TynFrame){&_dispose, &_step, &_draw};

  COLLISION_DRAW_DEBUG = false;

  return stage->state;
}

static void apply_body_force(GamePlatformerBody *body, Vector2 force) {
  Vector2 acceleration = Vector2Scale(force, 1 / body->mass);
  body->velocity = Vector2Add(body->velocity, acceleration);
}

static void step_body(GamePlatformer0State *state, GamePlatformerBody *body) {
	// gravity
  const Vector2 gravity = (Vector2){0, 0.98 * body->mass};
  apply_body_force(body, gravity);

	// drag forces
	float speed = Vector2Length(body->velocity);
	float fdrag = speed * speed * body->drag;
	Vector2 drag = Vector2Scale(Vector2Normalize(Vector2Negate(body->velocity)), fdrag);
	apply_body_force(body, drag);

  Vector2 position = Vector2Add(body->position, body->velocity);

  b2Vec2 box_extents = b2AABB_Extents(*body->aabb);
  b2AABB box_at_goal = b2AABB_ConstructFromCenterSize(
      position.x, position.y, box_extents.x * 2, box_extents.y * 2);
  b2AABB boardphase = b2AABB_Union(*body->aabb, box_at_goal);

  b2AABB boarphased_boxes[COLLIDERS_COUNT] = {0};
  b2AABB active_boxes[COLLIDERS_COUNT] = {0};
  int boarphased_boxes_count = 0;
  int active_boxes_count = 0;

  // Boxes size decreased a bit and collision tests shifts final positions a bit
  const int extend_margin = 1;
  for (int i = 0; i < COLLIDERS_COUNT; i++) {
    GamePlatformerCollider *collider = &state->colliders[i];
    b2AABB *aabb = &state->aabbs[i];

    if (!collider->active) {
      continue;
    }

    active_boxes[active_boxes_count++] = *aabb;

    if (b2AABB_Overlaps(boardphase, *aabb)) {
      b2AABB extended_box1 =
          b2AABB_ExtendBySize(*aabb, box_extents.x * 2 - extend_margin * 2,
                              box_extents.y * 2 - extend_margin * 2);
      boarphased_boxes[boarphased_boxes_count++] = extended_box1;
    }
  }

  // a. Simple collision - put collider out of any bounds
  b2Vec2 newpos = simpleAABBCollision(*body->aabb, &active_boxes,
                                      active_boxes_count, extend_margin);

  // b. swept collision - move collider in required direction
  int iterations = 0;
  b2Vec2 _target = (b2Vec2){position.x, position.y};
  newpos = sweptAABBCollision(&iterations, newpos, _target, &boarphased_boxes,
                              boarphased_boxes_count, extend_margin);

  body->position.x = newpos.x;
  body->position.y = newpos.y;
  b2AABB_SetPosition(body->aabb, body->position.x, body->position.y);

  // a. collision velocity discard, collision friction
  if (newpos.y != _target.y) {
    body->velocity.y = 0;
    body->velocity.x = body->velocity.x * (1 - body->friction);
  } if (newpos.x != _target.x) {
    body->velocity.x = 0;
    body->velocity.y = body->velocity.y * (1 - body->friction);
  }

}

static void _dispose(GamePlatformer0State *state) {
  MemFree(state->bodies);
  MemFree(state->colliders);
  MemFree(state->aabbs);
}

static STAGEFLAG _step(GamePlatformer0State *state, STAGEFLAG flags) {
  Vector2 acceleration = {0, 0};
  if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))
    acceleration.x += 1;
  if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))
    acceleration.x -= 1;
  if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
    acceleration.y -= 16;
  if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
  }

  apply_body_force(&state->pawn.body, acceleration);
  step_body(state, &state->pawn.body);

  return flags;
}

static void _draw(GamePlatformer0State *state) {
  drawb2AABBDebug(&state->pawn.aabb, RED);

  for (int i = 0; i < COLLIDERS_COUNT; i++) {
    GamePlatformerCollider *collider = &state->colliders[i];
    if (!collider->active) {
      continue;
    }

    drawb2AABBDebug(collider->aabb, BLACK);
  }
}
