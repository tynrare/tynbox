#include "include/test_physics_0.h"
#include "include/shader_sdf2d.h"
#include "body.h"
#include "shape.h"
#include "world.h"
#include "box2d/box2d.h"
#include "raymath.h"

static void _dispose(TestPhysics0State* state);
static STAGEFLAG _step(TestPhysics0State* state, STAGEFLAG flags);
static void _draw(TestPhysics0State* state);
static int MakeWorld(TestPhysics0State* state);
static b2BodyId MakeBodyBox(TestPhysics0State* state);
static b2BodyId MakeBodyBall(TestPhysics0State* state);

static void rayb2DrawShape(b2Shape* shape, b2Transform xf, Color color);
static void _drawShapes(TestPhysics0State* state);
static int rayb2DrawShapeRaymarch2d(b2Shape* shape, b2Transform xf, Color color, int shift);
static void _drawShapesRaymarch2d(TestPhysics0State* state);

static int mode = 1;

static void cmd(TestPhysics0State* state, char* command) {
	if (strcmp(command, "mode mix") == 0) {
		mode = 0;
	}
	else if (strcmp(command, "mode sdf") == 0) {
		mode = 1;
	}
	else if (strcmp(command, "mode vec") == 0) {
		mode = 2;
	}
}

TestPhysics0State* TestPhysics0Init(TynStage* stage) {
    TestPhysics0State* state = malloc(sizeof(TestPhysics0State));
    if (state == NULL) {
        return NULL;
    }

	Raymarch2dInit(&state->texdataset, &state->shader);
	MakeWorld(state);

	state->camera = (Camera2D){ 0 };
	state->camera.offset.y = GetScreenHeight();
	state->camera.offset.x = GetScreenWidth() / 2.0f;
	state->camera.zoom = -1;

    stage->state = state;
    stage->frame =
        (TynFrame){ &_dispose, &_step, &_draw, NULL, &cmd };

    return stage->state;
}

static void _dispose(TestPhysics0State* state)
{
	b2DestroyWorld(state->worldId);
	UnloadShader(state->shader);
	free(state);
}

static STAGEFLAG _step(TestPhysics0State* state, STAGEFLAG flags)
{
	// Prepare for simulation. Typically we use a time step of 1/60 of a
	// second (60Hz) and 10 iterations. This provides a high quality simulation
	// in most game scenarios.
	static const float dt = 1.0f / 60.0f;
	//float dt = GetFrameTime() * 2.0f;
	static const int32_t velocityIterations = 6;
	static const int32_t relaxIterations = 2;
	b2World_Step(state->worldId, dt, velocityIterations, relaxIterations);

	if (IsKeyPressed(KEY_A)) {
		MakeBodyBox(state);
	}
	else if (IsKeyPressed(KEY_S)) {
		for(int i = 0; i < 10; i++)
		MakeBodyBall(state);
	}
	return flags;
}



static void _draw(TestPhysics0State* state)
{
	if (mode == 0 || mode == 1) {
		Raymarch2dBeginWrite(&state->texdataset, true);
		_drawShapesRaymarch2d(state);

		Raymarch2dEndWrite();

		Raymarch2dDraw(&state->texdataset, &state->shader);
	}
	if (mode == 0 || mode == 2) {
		BeginMode2D(state->camera);

		_drawShapes(state);

		EndMode2D();
	}
}

static b2BodyId MakeBodyBox(TestPhysics0State* state) {
	// Define the dynamic body. We set its position and call the body factory.
	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.type = b2_dynamicBody;
	bodyDef.position = (b2Vec2){ 0.0f, 100.0f };
	b2BodyId bodyId = b2World_CreateBody(state->worldId, &bodyDef);

	// Define another box shape for our dynamic body.
	b2Polygon dynamicBox = b2MakeBox(10.0f, 10.0f);

	// Define the dynamic body shape
	b2ShapeDef shapeDef = b2DefaultShapeDef();

	// Set the box density to be non-zero, so it will be dynamic.
	shapeDef.density = 1.0f;
	shapeDef.restitution = 0.5f;

	// Override the default friction.
	shapeDef.friction = 0.3f;

	// Add the shape to the body.
	b2Body_CreatePolygon(bodyId, &shapeDef, &dynamicBox);

	return bodyId;
}

static b2BodyId MakeBodyBall(TestPhysics0State* state) {
	// Define the dynamic body. We set its position and call the body factory.
	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.type = b2_dynamicBody;
	bodyDef.position = (b2Vec2){ 0.0f, 100.0f };
	b2BodyId bodyId = b2World_CreateBody(state->worldId, &bodyDef);

	// Define another box shape for our dynamic body.
	b2Circle dynamicCircle = { b2MakeVec2(0, 0), 10.0f };

	// Define the dynamic body shape
	b2ShapeDef shapeDef = b2DefaultShapeDef();

	// Set the box density to be non-zero, so it will be dynamic.
	shapeDef.density = 1.0f;
	shapeDef.restitution = 0.5f;

	// Override the default friction.
	shapeDef.friction = 0.3f;

	// Add the shape to the body.
	b2Body_CreateCircle(bodyId, &shapeDef, &dynamicCircle);

	return bodyId;
}


static int MakeWorld(TestPhysics0State* state)
{
	// Define the gravity vector.
	b2Vec2 gravity = { 0.0f, -98.0f };

	// Construct a world object, which will hold and simulate the rigid bodies.
	b2WorldDef worldDef = b2DefaultWorldDef();
	worldDef.gravity = gravity;

	b2WorldId worldId = b2CreateWorld(&worldDef);

	// Define the ground body.
	b2BodyDef groundBodyDef = b2DefaultBodyDef();
	groundBodyDef.position = (b2Vec2){ 0.0f, 10.0f };

	// Call the body factory which allocates memory for the ground body
	// from a pool and creates the ground box shape (also from a pool).
	// The body is also added to the world.
	b2BodyId groundBodyId = b2World_CreateBody(worldId, &groundBodyDef);

	// Define the ground box shape. The extents are the half-widths of the box.
	b2Polygon groundBox = b2MakeBox(512.0f, 10.0f);

	// Add the box shape to the ground body.
	b2ShapeDef groundShapeDef = b2DefaultShapeDef();
	b2Body_CreatePolygon(groundBodyId, &groundShapeDef, &groundBox);

	state->worldId = worldId;
	state->groundBodyId = groundBodyId;

	MakeBodyBox(state);

	return 0;
}

static void rayb2DrawShape(b2Shape* shape, b2Transform xf, Color color)
{
	switch (shape->type)
	{
	case b2_capsuleShape:
	{
		b2Capsule* capsule = &shape->capsule;
		b2Vec2 p1 = b2TransformPoint(xf, capsule->point1);
		b2Vec2 p2 = b2TransformPoint(xf, capsule->point2);
		DrawCircleLines(p1.x, p1.y, capsule->radius, color);
		DrawCircleLines(p2.x, p2.y, capsule->radius, color);
	}
	break;

	case b2_circleShape:
	{
		b2Circle* circle = &shape->circle;
		b2Vec2 center = b2TransformPoint(xf, circle->point);
		b2Vec2 axis = b2RotateVector(xf.q, (b2Vec2) { 1.0f, 0.0f });
		DrawCircleLines(center.x, center.y, circle->radius, color);
		DrawLine(center.x, center.y, center.x + axis.x * circle->radius, center.y + axis.y * circle->radius, WHITE);
	}
	break;

	case b2_polygonShape:
	{
		b2Polygon* poly = &shape->polygon;
		int32_t count = poly->count;
		B2_ASSERT(count <= b2_maxPolygonVertices);
		b2Vec2 vertices[b2_maxPolygonVertices];

		for (int32_t i = 0; i < count; ++i)
		{
			vertices[i] = b2TransformPoint(xf, poly->vertices[i]);
		}

		for (int32_t i = 0; i < count; ++i) {
			if (poly->radius > 0.0f) {
				DrawLineBezier((Vector2) { vertices[i].x, vertices[i].y }, (Vector2) { vertices[(i + 1) % count].x, vertices[(i + 1) % count].y }, poly->radius, color);
			}
			else {
				DrawLine(vertices[i].x, vertices[i].y, vertices[(i + 1) % count].x, vertices[(i + 1) % count].y, color);
			}
		}
	}
	break;

	case b2_segmentShape:
	{
		b2Segment* segment = &shape->segment;
		b2Vec2 p1 = b2TransformPoint(xf, segment->point1);
		b2Vec2 p2 = b2TransformPoint(xf, segment->point2);
		DrawLine(p1.x, p1.y, p2.x, p2.y, color);
	}
	break;

	case b2_smoothSegmentShape:
	{
		b2Segment* segment = &shape->smoothSegment.segment;
		b2Vec2 p1 = b2TransformPoint(xf, segment->point1);
		b2Vec2 p2 = b2TransformPoint(xf, segment->point2);
		//draw->DrawSegment(p1, p2, color, draw->context);
		//draw->DrawPoint(p2, 4.0f, color, draw->context);
		//draw->DrawSegment(p1, b2Lerp(p1, p2, 0.1f), b2MakeColor(b2_colorPaleGreen4, 1.0f), draw->context);
	}
	break;

	default:
		break;
	}
}

static void _drawShapes(TestPhysics0State* state) {
	b2World* world = b2GetWorldFromId(state->worldId);
	int32_t count = world->bodyPool.capacity;
	for (int32_t i = 0; i < count; ++i)
	{
		b2Body* b = world->bodies + i;
		if (b->object.next != i)
		{
			continue;
		}

		bool isAwake = false;
		if (b->islandIndex != B2_NULL_INDEX)
		{
			isAwake = world->islands[b->islandIndex].awakeIndex != B2_NULL_INDEX;
		}

		b2Transform xf = b->transform;
		int32_t shapeIndex = b->shapeList;
		while (shapeIndex != B2_NULL_INDEX)
		{
			b2Shape* shape = world->shapes + shapeIndex;
			Color color;

			if (b->type == b2_dynamicBody && b->mass == 0.0f)
			{
				// Bad body
				color = RED;
			}
			else if (b->isEnabled == false)
			{
				color = GRAY;
			}
			else if (shape->isSensor)
			{
				color = PURPLE;
			}
			else if (b->isSpeedCapped)
			{
				color = YELLOW;
			}
			else if (b->isFast)
			{
				color = DARKBLUE;
			}
			else if (b->type == b2_staticBody)
			{
				color = GREEN;
			}
			else if (b->type == b2_kinematicBody)
			{
				color = MAGENTA;
			}
			else if (isAwake)
			{
				color = PINK;
			}
			else
			{
				color = DARKGRAY;
			}

			rayb2DrawShape(shape, xf, color);
			shapeIndex = shape->nextShapeIndex;
		}
	}
}

static int rayb2DrawShapeRaymarch2d(b2Body *body, b2Shape* shape, b2Transform xf, Color color, int shift)
{
	switch (shape->type)
	{
	case b2_capsuleShape:
	{
		b2Capsule* capsule = &shape->capsule;
		b2Vec2 p1 = b2TransformPoint(xf, capsule->point1);
		b2Vec2 p2 = b2TransformPoint(xf, capsule->point2);
		shift = Raymarch2dWriteEntity(shift, CIRCLE, (Vector2){ p1.x, p1.y }, (Vector2) {
			capsule->radius, capsule->radius
		}, 0);
		shift = Raymarch2dWriteEntity(shift, CIRCLE, (Vector2) { p1.x, p1.y }, (Vector2) {
			capsule->radius, capsule->radius
		}, 0);
		return shift;
	}
	break;

	case b2_circleShape:
	{
		b2Circle* circle = &shape->circle;
		b2Vec2 _center = b2TransformPoint(xf, circle->point);
		b2Vec2 axis = b2RotateVector(xf.q, (b2Vec2) { 1.0f, 0.0f });
		//DrawCircle(center.x, center.y, circle->radius, color);
		//DrawCircle(axis.x, axis.y, 1, BLACK);
		Vector2 center = (Vector2){ -_center.x, _center.y };
		center.x += GetScreenWidth() / 2;
		center.y = GetScreenHeight() - center.y;
		shift = Raymarch2dWriteEntity(shift, CIRCLE, center, (Vector2) {
			circle->radius, 0
		}, 0);
	}
	break;

	case b2_polygonShape:
	{
		b2AABB aabb = b2ComputePolygonAABB(&shape->polygon, (b2Transform){b2MakeVec2(0,0), b2MakeRot(0,0) });
		Vector2 size = (Vector2){ aabb.upperBound.x - aabb.lowerBound.x, aabb.upperBound.y - aabb.lowerBound.y };
		size = Vector2Scale(size, 0.5f);
		Vector2 center = (Vector2){ -xf.p.x, xf.p.y };
		center.x += GetScreenWidth() / 2;
		center.y = GetScreenHeight() - center.y;
		float angle = -b2Rot_GetAngle(xf.q) * RAD2DEG;
		//b2Vec2 axis = b2RotateVector(xf.q, (b2Vec2) { 1.0f, 0.0f });
		//float angle = -Vector2Angle((Vector2) { 1.0f, 0.0f }, 
		//	(Vector2) { axis.x, axis.y })* RAD2DEG;
		return Raymarch2dWriteEntity(shift, BOX, center, size, angle);
	}
	break;

	case b2_segmentShape:
	{
		b2Segment* segment = &shape->segment;
		b2Vec2 p1 = b2TransformPoint(xf, segment->point1);
		b2Vec2 p2 = b2TransformPoint(xf, segment->point2);
		//DrawLine(p1.x, p1.y, p2.x, p2.y, color);
	}
	break;

	case b2_smoothSegmentShape:
	{
		b2Segment* segment = &shape->smoothSegment.segment;
		b2Vec2 p1 = b2TransformPoint(xf, segment->point1);
		b2Vec2 p2 = b2TransformPoint(xf, segment->point2);
		//draw->DrawSegment(p1, p2, color, draw->context);
		//draw->DrawPoint(p2, 4.0f, color, draw->context);
		//draw->DrawSegment(p1, b2Lerp(p1, p2, 0.1f), b2MakeColor(b2_colorPaleGreen4, 1.0f), draw->context);
	}
	break;

	default:
		break;
	}

	return shift;
}

static void _drawShapesRaymarch2d(TestPhysics0State* state) {
	b2World* world = b2GetWorldFromId(state->worldId);
	int32_t count = world->bodyPool.capacity;
	int shift = 0;
	for (int32_t i = 0; i < count; ++i)
	{
		b2Body* b = world->bodies + i;
		if (b->object.next != i)
		{
			continue;
		}

		bool isAwake = false;
		if (b->islandIndex != B2_NULL_INDEX)
		{
			isAwake = world->islands[b->islandIndex].awakeIndex != B2_NULL_INDEX;
		}

		b2Transform xf = b->transform;
		int32_t shapeIndex = b->shapeList;
		while (shapeIndex != B2_NULL_INDEX)
		{
			b2Shape* shape = world->shapes + shapeIndex;
			Color color;

			if (b->type == b2_dynamicBody && b->mass == 0.0f)
			{
				// Bad body
				color = RED;
			}
			else if (b->isEnabled == false)
			{
				color = GRAY;
			}
			else if (shape->isSensor)
			{
				color = PURPLE;
			}
			else if (b->isSpeedCapped)
			{
				color = YELLOW;
			}
			else if (b->isFast)
			{
				color = DARKBLUE;
			}
			else if (b->type == b2_staticBody)
			{
				color = GREEN;
			}
			else if (b->type == b2_kinematicBody)
			{
				color = MAGENTA;
			}
			else if (isAwake)
			{
				color = PINK;
			}
			else
			{
				color = DARKGRAY;
			}

			shift = rayb2DrawShapeRaymarch2d(b, shape, xf, color, shift);
			shapeIndex = shape->nextShapeIndex;
		}
	}
}
