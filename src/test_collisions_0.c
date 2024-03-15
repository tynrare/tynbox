#include "include/test_collisions_0.h"
#include "include/collisions.h"
#include <stdlib.h>

static void _dispose(TestCollisions0State* state);
static STAGEFLAG _step(TestCollisions0State* state, STAGEFLAG flags);
static void _draw(TestCollisions0State* state);

TestCollisions0State* TestCollisions0Init(TynStage* stage) {
    TestCollisions0State* state = malloc(sizeof(TestCollisions0State));
    if (state == NULL) {
        return NULL;
    }

    state->testbox_posx = 16;
    state->testbox_posy = 16;

    stage->state = state;
    stage->frame =
        (TynFrame){ &_dispose, &_step, &_draw };

		COLLISION_DRAW_DEBUG = true;

    return stage->state;
}

void _dispose(TestCollisions0State* state)
{
}

STAGEFLAG _step(TestCollisions0State* state, STAGEFLAG flags)
{
    return flags;
}

void _draw(TestCollisions0State* state)
{
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();
    int mx = state->testbox_posx;
    int my = state->testbox_posy;
    bool use_keyboard = IsKeyDown(KEY_A) || IsKeyDown(KEY_D) || IsKeyDown(KEY_W) || IsKeyDown(KEY_S);
    bool apply_movement = IsMouseButtonDown(MOUSE_BUTTON_RIGHT) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || use_keyboard;

    const int keboard_movement_speed = 3;
    if (IsKeyDown(KEY_A)) {
        mx -= keboard_movement_speed;
    } else if (IsKeyDown(KEY_D)) {
        mx += keboard_movement_speed;
    }
    if (IsKeyDown(KEY_W)) {
        my -= keboard_movement_speed;
    } else if (IsKeyDown(KEY_S)) {
        my += keboard_movement_speed;
    }

    if (!use_keyboard) {
        mx = GetMouseX();
        my = GetMouseY();
    }

    // mouse follow box
    b2AABB mainbox = b2AABB_ConstructFromCenterSize(state->testbox_posx, state->testbox_posy, 32, 32);
    b2Vec2 box_pos = b2AABB_Center(mainbox);
    b2Vec2 box_extents = b2AABB_Extents(mainbox);
    b2Vec2 mouse_pos = { mx, my };

    b2AABB box_at_goal = b2AABB_ConstructFromCenterSize(mx, my, box_extents.x * 2, box_extents.y * 2);
    b2AABB boardphase = b2AABB_Union(mainbox, box_at_goal);

    // test boxes
    b2AABB testboxes[32] = { 0 };
    const int testboxes_origin_x = sw / 2 - 4 * 32 + 16;
    const int testboxes_origin_y = sh / 2;
    for (int i = 0; i < 32; i++) {
        float x = testboxes_origin_x + (((i / 8) + i * 2) % 8) * 32 - 16;
        float y = testboxes_origin_y + (i / 8) * 32 - 16;
        y += (((i / 8) % 2) ? sin(GetTime() * 3) : cos(GetTime() * 3)) * 32;
        b2AABB box = b2AABB_ConstructFromCenterSize(x, y, 32, 32);
        testboxes[i] = box;
    }

    // we can find boardphase collides only once - no more gonna be included
    // in other hand refinding colliders in smaller area could save raycast time
    b2AABB boarphased_boxes[32] = { 0 };
    int boarphased_boxes_count = 0;

    // Boxes size decreased a bit and collision tests shifts final positions a bit
    const int extend_margin = 1;
    for (int i = 0; i < 32; i++) {
        b2AABB box = testboxes[i];

        if (b2AABB_Overlaps(boardphase, box)) {
            b2AABB extended_box1 = b2AABB_ExtendBySize(box, box_extents.x * 2 - extend_margin * 2, box_extents.y * 2 - extend_margin * 2);
            boarphased_boxes[boarphased_boxes_count++] = extended_box1;

            drawb2AABBDebug(&box, BLACK); // test box
        }
        else {
            drawb2AABBDebug(&box, GRAY); // test box
        }
    }

    // a. Simple collision - put collider out of any bounds
    b2Vec2 newpos = simpleAABBCollision(mainbox, testboxes, 32, extend_margin);
    state->testbox_posx = newpos.x;
    state->testbox_posy = newpos.y;

    // b. swept collision - move collider in required direction
    int iterations = 0;
    newpos = sweptAABBCollision(&iterations, newpos, mouse_pos, &boarphased_boxes, boarphased_boxes_count, extend_margin);

    if (apply_movement) {
        state->testbox_posx = newpos.x;
        state->testbox_posy = newpos.y;
    }

    //DrawText(TextFormat("%d iterations", iterations), 16, 16, 12, BLACK);

    drawb2AABBDebug(&mainbox, BLACK); // main (moving) box
		b2AABB _box = b2AABB_ConstructFromCenterSize(newpos.x, newpos.y, box_extents.x * 2, box_extents.y * 2);
    drawb2AABBDebug(&_box, GRAY);
    drawb2AABBDebug(&boardphase, YELLOW); // moving box boardphase
}
