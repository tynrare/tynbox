#include "include/test_networksim_0.h"
#include <stdlib.h>
#include <assert.h>
#include <raymath.h>
#include <math.h>

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
    float progress_len = max(0, progress * path->total_length);

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

    state->inputonce = false;

    state->move.timestamp = GetTime();
    state->move.speed = 50;
    state->move.path = makePath(3);
    writePathPoint(&state->move.path, (Vector2) { 50, 50 });
    writePathPoint(&state->move.path, (Vector2) { 30, 30 });
    writePathPoint(&state->move.path, (Vector2) { 50, 100 });

    stage->state = state;
    stage->frame =
        (TynFrame){ &TestNetworksim0Dispose, &TestNetworksim0Step, &TestNetworksim0Draw };

    return stage->state;
}

void TestNetworksim0Dispose(TestNetworksim0State* state)
{
    free(state);
}

float getMoveProgress(TestNetworksim0MoveAction* move) {
    double now = GetTime();
    double delta = now - move->timestamp;
    float progress = delta * move->speed / move->path.total_length;

    return progress;
}

STAGEFLAG TestNetworksim0Step(TestNetworksim0State* state, STAGEFLAG flags)
{
    if (getMoveProgress(&state->move) >= 1) {
        state->move.timestamp = GetTime();
    }


    bool inputed = false;

    if (isAnyKeyPressed(1, KEY_E)) {
        inputed = true;
    }
    else {
        state->inputonce = true;
    }

    if (state->inputonce) {
        // inputs
    }

    if (inputed) {
        state->inputonce = false;
    }

    return flags;
}

void TestNetworksim0Draw(TestNetworksim0State* state)
{
    DrawText(TextFormat("Test network sim."),
        10, 10, 10, BLACK);

    TestNetworksim0Path path = state->move.path;

    float progress = getMoveProgress(&state->move);
    Vector2 pos = getPathPos(&path, progress);

    DrawText(TextFormat("Progress: %.2f", progress),
        10, 22, 10, BLACK);

    DrawCircle(pos.x, pos.y, 8, GREEN);

    for (int i = 0; i < path.points_count; i++) {
        Vector2 p = path.points[i];

        if (i > 0) {
            Vector2 p1 = path.points[i - 1];
            DrawLine(p1.x, p1.y, p.x, p.y, BLUE);
        }

        DrawCircle(p.x, p.y, 4, RED);
    }
}
