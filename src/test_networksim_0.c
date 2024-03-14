#include "include/test_networksim_0.h"
#include <stdlib.h>
#include <assert.h>
#include <raymath.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

void TestNetworksim0Dispose(TestNetworksim0State* state);
STAGEFLAG TestNetworksim0Step(TestNetworksim0State* state, STAGEFLAG flags);
void TestNetworksim0Draw(TestNetworksim0State* state);

// --- utils

TestNetworksim0Path makePath(short pcount) {
    TestNetworksim0Path path = {
        malloc(sizeof(Vector2) * pcount),       // *points
        malloc(sizeof(float) * (pcount - 1)),   // *segments_length
        0,                                      // total_length
        0,                                      // points_count
    };

    return path;
}

void writePathPoint(TestNetworksim0Path *path, Vector2 point) {
    path->points[path->points_count] = point;

    if (path->points_count > 0) {
        float len = Vector2Distance(path->points[path->points_count], path->points[path->points_count - 1]);
        path->segments_length[path->points_count - 1] = len;
        path->total_length += len;
    }

    path->points_count += 1;
}

Vector2 getPathPos(TestNetworksim0Path* path, float progress) {
    assert(("Path requires at least 2 points", path->points_count > 1));

    float len = 0;
    float progress_len = fmax(0, progress * path->total_length);

    for (int i = 0; i < path->points_count - 1; i++) {
        float segment_len = path->segments_length[i];
        float nextlen = len + segment_len;

        if (nextlen > progress_len) {
            float segment_progress = (progress_len - len) / segment_len;
            Vector2 segment = Vector2Subtract(path->points[i + 1], path->points[i]);
            Vector2 segment_normalized = Vector2Normalize(segment);
            Vector2 segment_pos = Vector2Scale(segment_normalized, segment_progress * segment_len);
            Vector2 pos = Vector2Add(path->points[i], segment_pos);

            return pos;
        }

        len = nextlen;
    }

    return path->points[path->points_count - 1];
}


// --- stage logic

TestNetworksim0State* TestNetworksim0Init(TynStage* stage) {
    TestNetworksim0State* state = malloc(sizeof(TestNetworksim0State));

    state->move1.timestamp = GetTime();
    state->move1.speed = 50;
    state->move1.path = makePath(5);
    writePathPoint(&state->move1.path, (Vector2) { 50, 50 });
    writePathPoint(&state->move1.path, (Vector2) { 30, 30 });
    writePathPoint(&state->move1.path, (Vector2) { 50, 100 });
    writePathPoint(&state->move1.path, (Vector2) { 0, 300 });
    writePathPoint(&state->move1.path, (Vector2) { 100, 70 });
    memcpy(&state->move2, &state->move1, sizeof(TestNetworksim0MoveAction));

    stage->state = state;
    state->elapsed = 0;
    stage->frame =
        (TynFrame){ &TestNetworksim0Dispose, &TestNetworksim0Step, &TestNetworksim0Draw };

    return stage->state;
}

void TestNetworksim0Dispose(TestNetworksim0State* state)
{
    free(state);
}

float getMoveProgress(TestNetworksim0MoveAction* move1, double now) {
    double delta = now - move1->timestamp;
    float progress = delta * move1->speed / move1->path.total_length;

    return progress;
}

STAGEFLAG TestNetworksim0Step(TestNetworksim0State* state, STAGEFLAG flags)
{
    if (getMoveProgress(&state->move1, GetTime()) >= 1) {
        state->move1.timestamp = GetTime();
    }
    if (getMoveProgress(&state->move2, GetTime()) >= 1) {
        state->move2.timestamp = GetTime();
    }

    return flags;
}

static void drawPath(TestNetworksim0MoveAction* move, Vector2 origin, double now) {
    TestNetworksim0Path path = move->path;

    float progress = getMoveProgress(move, now);
    Vector2 pos = getPathPos(&path, progress);

    DrawText(TextFormat("Progress: %.2f", progress),
        10 + origin.x, 22 + origin.y, 10, BLACK);

    DrawCircle(pos.x + origin.x, pos.y + origin.y, 8, GREEN);

    for (int i = 0; i < path.points_count; i++) {
        Vector2 p = path.points[i];

        if (i > 0) {
            Vector2 p1 = path.points[i - 1];
            DrawLine(p1.x + origin.x, p1.y + origin.y, p.x + origin.x, p.y + origin.y, BLUE);
        }

        DrawCircle(p.x + origin.x, p.y + origin.y, 4, RED);
    }
}

void TestNetworksim0Draw(TestNetworksim0State* state)
{
    DrawText(TextFormat("Test network sim."),
        10, 10, 10, BLACK);

    // ref path
    double now = GetTime();
    drawPath(&state->move1, (Vector2) { 0, 0 }, now);

    // "lagged" path
    double tt = 0;
    const double threshold = 300;
    double _cthreshold = 1000 / threshold;

    double d = modf(now * _cthreshold, &tt);
    double _tnow = tt / _cthreshold;
    Vector2 path2_origin = (Vector2){ 100, 0 };
    drawPath(&state->move2, path2_origin, _tnow);

    float progress2 = getMoveProgress(&state->move2, _tnow + (now - _tnow));
    Vector2 pos2 = getPathPos(&state->move2.path, progress2);
    DrawCircle(pos2.x + path2_origin.x, pos2.y + path2_origin.y, 4, BLUE);
}
