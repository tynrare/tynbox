#pragma once

#include "../external/box2c/types.h"
#include "../external/box2c/aabb.h"
#include "../external/box2c/math.h"
#include <raylib.h>

#ifndef COLLISIONS_H
#define COLLISIONS_H

static bool COLLISION_DRAW_DEBUG = true;

static b2AABB b2AABB_ConstructFromCenterSize(int x, int y, int w, int h) {
    int halfwidth = w / 2;
    int halfheight = h / 2;
    b2AABB aabb = { (b2Vec2) { x - halfwidth, y - halfheight }, (b2Vec2) { x + halfwidth, y + halfheight } };

    return aabb;
}

static void b2AABB_Set(b2AABB *aabb, int x, int y, int w, int h) {
    int halfwidth = w / 2;
    int halfheight = h / 2;
    aabb->lowerBound.x = x - halfwidth;
    aabb->lowerBound.y = y - halfheight;
    aabb->upperBound.x = x + halfwidth;
    aabb->upperBound.y = y + halfheight;
}

static void b2AABB_SetPosition(b2AABB *aabb, int x, int y) {
		int width = aabb->upperBound.x - aabb->lowerBound.x;
		int height = aabb->upperBound.y - aabb->lowerBound.y;
    int halfwidth = width / 2;
    int halfheight = height / 2;

    aabb->lowerBound.x = x - halfwidth;
    aabb->lowerBound.y = y - halfheight;
    aabb->upperBound.x = x + halfwidth;
    aabb->upperBound.y = y + halfheight;
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

static void drawb2AABBDebug(b2AABB *aabb, Color color) {
    b2Vec2 aabb_size = b2AABB_Extents(*aabb);
    DrawRectangleLines(aabb->lowerBound.x, aabb->lowerBound.y, aabb_size.x * 2, aabb_size.y * 2, color);
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

						if (COLLISION_DRAW_DEBUG) {
							DrawCircle(ro.point.x, ro.point.y, 4, RED);
							DrawLine(ro.point.x, ro.point.y, ro.point.x + ro.normal.x * 4, ro.point.y + ro.normal.y * 4, RED);
							drawb2AABBDebug(&box, GRAY); // test box extended
						}
        }
    }

    b2Vec2 newpos = { 0, 0 };

    newpos.x = raycast_closest.hit ? raycast_closest.point.x + raycast_closest.normal.x * extend_margin : target.x;
    newpos.y = raycast_closest.hit ? raycast_closest.point.y + raycast_closest.normal.y * extend_margin : target.y;

    if (!raycast_closest.hit) {
				if (COLLISION_DRAW_DEBUG) {
					DrawLine(pos.x, pos.y, target.x, target.y, BLACK); // rayline
				}

        return newpos;
    } else if (COLLISION_DRAW_DEBUG) {
        DrawLine(pos.x, pos.y, newpos.x, newpos.y, GREEN); // rayline
        DrawLine(newpos.x, newpos.y, target.x, target.y, RED); // rayline

        DrawCircle(raycast_closest.point.x, raycast_closest.point.y, 8, GREEN);
        DrawLine(raycast_closest.point.x, raycast_closest.point.y, raycast_closest.point.x + raycast_closest.normal.x * 16, raycast_closest.point.y + raycast_closest.normal.y * 16, GREEN);
        //DrawText(TextFormat("%d ", *iteration), newpos.x - 2, newpos.y - 6, 14, BLACK);
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


#endif
