#include "include/test_collisions_0.h"
#include "external/box2c/types.h"
#include "external/box2c/aabb.h"
#include "external/box2c/math.h"
#include <stdlib.h>

static void _dispose(TestCollisions0State* state);
static STAGEFLAG _step(TestCollisions0State* state, STAGEFLAG flags);
static void _draw(TestCollisions0State* state);

static b2AABB b2AABB_ConstructFromCenterSize(int x, int y, int w, int h) {
    int halfwidth = w / 2;
    int halfheight = h / 2;
    b2AABB aabb = { (b2Vec2) { x - halfwidth, y - halfheight }, (b2Vec2) { x + halfwidth, y + halfheight } };

    return aabb;
}

static b2AABB b2AABB_ExtendBySize(b2AABB a, int w, int h)
{
    b2AABB c;
    c.lowerBound.x = a.lowerBound.x - w / 2;
    c.lowerBound.y = a.lowerBound.y - h / 2;
    c.upperBound.x = a.upperBound.x + w / 2;
    c.upperBound.y = a.upperBound.y + h / 2;
    return c;
}

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

    return stage->state;
}

void _dispose(TestCollisions0State* state)
{
}

STAGEFLAG _step(TestCollisions0State* state, STAGEFLAG flags)
{
    return flags;
}

static void drawb2AABBDebug(b2AABB aabb, Color color) {
    b2Vec2 aabb_size = b2AABB_Extents(aabb);
    DrawRectangleLines(aabb.lowerBound.x, aabb.lowerBound.y, aabb_size.x * 2, aabb_size.y * 2, color);
}

/**
* @param {int*} iteration total iterations count
* @param {b2Vec2} pos staring ray pos
* @param {b2Vec2} target goal ray pos
* @param {Array<b2AABB>*} colliders to check collision with. Has to be extended
* @param {int} colliders_count colliders array length
* @param {int} extend_margin additional margint shift to apply to final point
* @returns {b2Vec2} new pos that should be set
*/
static b2Vec2 sweptAABBCollision(int *iteration, b2Vec2 pos, b2Vec2 target, b2AABB *colliders, int colliders_count, int extend_margin) {
    *iteration += 1;

    // this whole process has to be recursive
    b2RayCastOutput raycast_closest = { 0 }; // closest ro;
    for (int i = 0; i < colliders_count; i++) {
        b2AABB box = colliders[i];

        // does not work from inside
        b2RayCastOutput ro = b2AABB_RayCast(box, pos, target);

        // main issue right now: ray clips on corners
        if (ro.hit) {
            if (!raycast_closest.hit || raycast_closest.fraction > ro.fraction) {
                raycast_closest = ro;
            }

            DrawCircle(ro.point.x, ro.point.y, 4, RED);
            DrawLine(ro.point.x, ro.point.y, ro.point.x + ro.normal.x * 4, ro.point.y + ro.normal.y * 4, RED);
            drawb2AABBDebug(box, GRAY); // test box extended
        }
    }

    b2Vec2 newpos = { 0, 0 };

    newpos.x = raycast_closest.hit ? raycast_closest.point.x + raycast_closest.normal.x * extend_margin : target.x;
    newpos.y = raycast_closest.hit ? raycast_closest.point.y + raycast_closest.normal.y * extend_margin : target.y;

    if (!raycast_closest.hit) {
        DrawLine(pos.x, pos.y, target.x, target.y, BLACK); // rayline

        return newpos;
    } else {
        DrawLine(pos.x, pos.y, newpos.x, newpos.y, GREEN); // rayline
        DrawLine(newpos.x, newpos.y, target.x, target.y, RED); // rayline

        DrawCircle(raycast_closest.point.x, raycast_closest.point.y, 8, GREEN);
        DrawLine(raycast_closest.point.x, raycast_closest.point.y, raycast_closest.point.x + raycast_closest.normal.x * 16, raycast_closest.point.y + raycast_closest.normal.y * 16, GREEN);
        DrawText(TextFormat("%d ", *iteration), newpos.x - 2, newpos.y - 6, 14, BLACK);
    }

    b2Vec2 dir = { target.x - newpos.x, target.y - newpos.y };
    b2Vec2 crossnormal = { raycast_closest.normal.y, raycast_closest.normal.x }; // swapped axes
    float newdirlen = b2Dot(dir, crossnormal);
    b2Vec2 newdir = b2MulSV(newdirlen, crossnormal);
    b2Vec2 newtarg = b2Add(newpos, newdir);

    if (newdirlen != 0) {
        return sweptAABBCollision(iteration, newpos, newtarg, colliders, colliders_count, extend_margin);
    }
   
    return newpos;
}

static b2Vec2 simpleAABBCollision(b2AABB collider, b2AABB* colliders, int colliders_count, int extend_margin) {
    b2AABB a = collider;
    b2Vec2 extents = b2AABB_Extents(collider);
    b2Vec2 pos = b2AABB_Center(collider);
    for (int i = 0; i < colliders_count; i++) {
        b2AABB b = colliders[i];
        b2Vec2 bpos = b2AABB_Center(b);
        b2Vec2 bextents = b2AABB_Extents(b);



        b2Vec2 d1 = { b.lowerBound.x - (pos.x + extents.x), b.lowerBound.y - (pos.y + extents.y) };
        b2Vec2 d2 = { (pos.x - extents.x) - b.upperBound.x, (pos.y - extents.y) - b.upperBound.y };

        if (d1.x > 0.0f || d1.y > 0.0f)
            continue;

        if (d2.x > 0.0f || d2.y > 0.0f)
            continue;

        int x = -B2_MAX(d1.x, d2.x);
        int y = -B2_MAX(d1.y, d2.y);
        int normal_y = d1.y > d2.y ? 1 : -1;
        int normal_x = d1.x > d2.x ? 1 : -1;

        if (x < y) {
            pos.x -= x * normal_x;
        }
        else if (x > y) {
            pos.y -= y * normal_y;
        }
        else {
            pos.x -= x * normal_x;
            pos.y -= y * normal_y;
        }
    }

    return pos;
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

            drawb2AABBDebug(box, BLACK); // test box
        }
        else {
            drawb2AABBDebug(box, GRAY); // test box
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

    drawb2AABBDebug(mainbox, BLACK); // main (moving) box
    drawb2AABBDebug(b2AABB_ConstructFromCenterSize(newpos.x, newpos.y, box_extents.x * 2, box_extents.y * 2), GRAY);
    drawb2AABBDebug(boardphase, YELLOW); // moving box boardphase
}
