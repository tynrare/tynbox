#include "include/test_collisions_0.h"
#include "box2d/types.h";
#include "box2d/aabb.h";
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

void _draw(TestCollisions0State* state)
{
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();
    int mx = GetMouseX();
    int my = GetMouseY();
    bool apply_movement = IsMouseButtonDown(MOUSE_BUTTON_RIGHT) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsKeyDown(KEY_A) || IsKeyDown(KEY_D) || IsKeyDown(KEY_W) || IsKeyDown(KEY_S);

    if (IsKeyDown(KEY_A)) {
        mx = state->testbox_posx - 1;
        my = state->testbox_posy;
    } else if (IsKeyDown(KEY_D)) {
        mx = state->testbox_posx + 1;
        my = state->testbox_posy;
    }
    if (IsKeyDown(KEY_W)) {
        mx = state->testbox_posx;
        my = state->testbox_posy - 1;
    } else if (IsKeyDown(KEY_S)) {
        mx = state->testbox_posx;
        my = state->testbox_posy + 1;
    }

    

    // mouse follow box
    b2AABB mainbox = b2AABB_ConstructFromCenterSize(state->testbox_posx, state->testbox_posy, 32, 32);
    b2Vec2 box_pos = b2AABB_Center(mainbox);
    b2Vec2 box_extents = b2AABB_Extents(mainbox);
    b2Vec2 mouse_pos = { mx, my };

    b2AABB box_at_goal = b2AABB_ConstructFromCenterSize(mx, my, box_extents.x * 2, box_extents.y * 2);
    b2AABB boardphase = b2AABB_Union(mainbox, box_at_goal);

    // test boxes
    b2AABB testboxes[16] = { 0 };
    const testboxes_origin_x = sw / 2 - 4 * 32 + 16;
    const testboxes_origin_y = sh / 2;
    for (int i = 0; i < 16; i++) {
        b2AABB box = b2AABB_ConstructFromCenterSize(testboxes_origin_x + (i % 8) * 32 - 16, testboxes_origin_y + (i / 8) * 32 - 16, 32, 32);
        testboxes[i] = box;
    }

    b2AABB boarphased_boxes[16] = { 0 };
    int boarphased_boxes_count = 0;
    for (int i = 0; i < 16; i++) {
        b2AABB box = testboxes[i];

        if (b2AABB_Overlaps(boardphase, box)) {
            b2AABB extended_box1 = b2AABB_ExtendBySize(box, box_extents.x * 2, box_extents.y * 2);
            boarphased_boxes[boarphased_boxes_count++] = extended_box1;

            drawb2AABBDebug(box, BLACK); // test box
        }
        else {
            drawb2AABBDebug(box, GRAY); // test box
        }
    }

    b2RayCastOutput raycasts[16] = { 0 };
    b2RayCastOutput raycast_closest = { 0 }; // closest ro;
    int raycasts_count = 0;
    for (int i = 0; i < boarphased_boxes_count; i++) {
        b2AABB box = boarphased_boxes[i];

        // does not work from inside
        b2RayCastOutput ro = b2AABB_RayCast(box, box_pos, mouse_pos);

        // main issue right now: ray clips on corners
        if (ro.hit) {
            raycasts[raycasts_count++] = ro;
            if (!raycast_closest.hit || raycast_closest.fraction > ro.fraction) {
                raycast_closest = ro;
            }

            DrawCircle(ro.point.x, ro.point.y, 2, GREEN);
            DrawLine(ro.point.x, ro.point.y, ro.point.x + ro.normal.x * 4, ro.point.y + ro.normal.y * 4, GREEN);
            drawb2AABBDebug(box, GRAY); // test box extended
        }
    }

    if (apply_movement) {
        state->testbox_posx = raycast_closest.hit ? raycast_closest.point.x : mx;
        state->testbox_posy = raycast_closest.hit ? raycast_closest.point.y : my;
    }

    DrawText(TextFormat("%f fraction", raycast_closest.fraction), 16, 16, 12, BLACK);

    drawb2AABBDebug(mainbox, BLACK); // main (moving) box
    drawb2AABBDebug(boardphase, YELLOW); // moving box boardphase
    DrawLine(box_pos.x, box_pos.y, mouse_pos.x, mouse_pos.y, raycast_closest.hit ? GREEN : RED); // rayline
    if (raycast_closest.hit) {
        DrawCircle(raycast_closest.point.x, raycast_closest.point.y, 4, GREEN);
        DrawLine(raycast_closest.point.x, raycast_closest.point.y, raycast_closest.point.x + raycast_closest.normal.x * 8, raycast_closest.point.y + raycast_closest.normal.y * 8, GREEN);
    }
}
