/**
This is network simulation for path finding based movement
*/
#include "tynroar_lib.h"
#include "raylib.h"

#ifndef TEST_NETWORKSIM0_H
#define TEST_NETWORKSIM0_H

typedef struct {
	Vector2 *points;
	float *segments_length;
	float total_length;
	short points_count;
} TestNetworksim0Path;

typedef struct {
	double timestamp;
	float speed; // pixels per second
	TestNetworksim0Path path;
} TestNetworksim0MoveAction;

typedef struct {
	double elapsed;
	TestNetworksim0MoveAction move1;
	TestNetworksim0MoveAction move2;
} TestNetworksim0State;

TestNetworksim0State* TestNetworksim0Init(TynStage* stage);

#endif

// todo:
/*high latency
variable latency
packet loss

rewind on collision
*/
