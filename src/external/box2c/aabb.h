// SPDX-FileCopyrightText: 2023 Erin Catto
// SPDX-License-Identifier: MIT

#pragma once

#include "constants.h"
#include "math.h"
#include "types.h"

// TODO_ERIN a lot of this could be internal

#define FLT_EPSILON      1.192092896e-07F        // smallest such that 1.0+FLT_EPSILON != 1.0
#define FLT_MAX          3.402823466e+38F        // max value

#ifdef __cplusplus
extern "C"
{
#endif

/// Verify that the bounds are sorted.
static inline bool b2AABB_IsValid(b2AABB a)
{
	b2Vec2 d = b2Sub(a.upperBound, a.lowerBound);
	bool valid = d.x >= 0.0f && d.y >= 0.0f;
	valid = valid && b2IsValidVec2(a.lowerBound) && b2IsValidVec2(a.upperBound);
	return valid;
}

/// Ray cast an AABB
// From Real-time Collision Detection, p179.
static inline b2RayCastOutput b2AABB_RayCast(b2AABB a, b2Vec2 p1, b2Vec2 p2)
{
	// Radius not handled
	b2RayCastOutput output = {0};

	float tmin = -FLT_MAX;
	float tmax = FLT_MAX;

	b2Vec2 p = p1;
	b2Vec2 d = b2Sub(p2, p1);
	b2Vec2 absD = b2Abs(d);

	b2Vec2 normal = b2Vec2_zero;

	// x-coordinate
	if (absD.x < FLT_EPSILON)
	{
		// parallel
		if (p.x < a.lowerBound.x || a.upperBound.x < p.x)
		{
			return output;
		}
	}
	else
	{
		float inv_d = 1.0f / d.x;
		float t1 = (a.lowerBound.x - p.x) * inv_d;
		float t2 = (a.upperBound.x - p.x) * inv_d;

		// Sign of the normal vector.
		float s = -1.0f;

		if (t1 > t2)
		{
			float tmp = t1;
			t1 = t2;
			t2 = tmp;
			s = 1.0f;
		}

		// Push the min up
		if (t1 > tmin)
		{
			normal.y = 0.0f;
			normal.x = s;
			tmin = t1;
		}

		// Pull the max down
		tmax = B2_MIN(tmax, t2);

		if (tmin > tmax)
		{
			return output;
		}
	}

	// y-coordinate
	if (absD.y < FLT_EPSILON)
	{
		// parallel
		if (p.y < a.lowerBound.y || a.upperBound.y < p.y)
		{
			return output;
		}
	}
	else
	{
		float inv_d = 1.0f / d.y;
		float t1 = (a.lowerBound.y - p.y) * inv_d;
		float t2 = (a.upperBound.y - p.y) * inv_d;

		// Sign of the normal vector.
		float s = -1.0f;

		if (t1 > t2)
		{
			float tmp = t1;
			t1 = t2;
			t2 = tmp;
			s = 1.0f;
		}

		// Push the min up
		if (t1 > tmin)
		{
			normal.x = 0.0f;
			normal.y = s;
			tmin = t1;
		}

		// Pull the max down
		tmax = B2_MIN(tmax, t2);

		if (tmin > tmax)
		{
			return output;
		}
	}
	
	// Does the ray start inside the box?
	// Does the ray intersect beyond the max fraction?
	if (tmin < 0.0f || 1.0f < tmin)
	{
		return output;
	}

	// Intersection.
	output.fraction = tmin;
	output.normal = normal;
	output.point = b2Lerp(p1, p2, tmin);
	output.hit = true;
	return output;
}

/// Get the center of the AABB.
static inline b2Vec2 b2AABB_Center(b2AABB a)
{
	b2Vec2 b = {0.5f * (a.lowerBound.x + a.upperBound.x), 0.5f * (a.lowerBound.y + a.upperBound.y)};
	return b;
}

/// Get the extents of the AABB (half-widths).
static inline b2Vec2 b2AABB_Extents(b2AABB a)
{
	b2Vec2 b = {0.5f * (a.upperBound.x - a.lowerBound.x), 0.5f * (a.upperBound.y - a.lowerBound.y)};
	return b;
}

/// Get the perimeter length
static inline float b2AABB_Perimeter(b2AABB a)
{
	float wx = a.upperBound.x - a.lowerBound.x;
	float wy = a.upperBound.y - a.lowerBound.y;
	return 2.0f * (wx + wy);
}

/// Union of two AABBs
static inline b2AABB b2AABB_Union(b2AABB a, b2AABB b)
{
	b2AABB c;
	c.lowerBound.x = B2_MIN(a.lowerBound.x, b.lowerBound.x);
	c.lowerBound.y = B2_MIN(a.lowerBound.y, b.lowerBound.y);
	c.upperBound.x = B2_MAX(a.upperBound.x, b.upperBound.x);
	c.upperBound.y = B2_MAX(a.upperBound.y, b.upperBound.y);
	return c;
}

/// Fatten an AABB
static inline b2AABB b2AABB_Extend(b2AABB a)
{
	b2AABB c;
	c.lowerBound.x = a.lowerBound.x - b2_aabbMargin;
	c.lowerBound.y = a.lowerBound.y - b2_aabbMargin;
	c.upperBound.x = a.upperBound.x + b2_aabbMargin;
	c.upperBound.y = a.upperBound.y + b2_aabbMargin;
	return c;
}

/// Enlarge a to contain b
/// @return true if the AABB grew
static inline bool b2AABB_Enlarge(b2AABB* a, b2AABB b)
{
	bool changed = false;
	if (b.lowerBound.x < a->lowerBound.x)
	{
		a->lowerBound.x = b.lowerBound.x;
		changed = true;
	}

	if (b.lowerBound.y < a->lowerBound.y)
	{
		a->lowerBound.y = b.lowerBound.y;
		changed = true;
	}

	if (a->upperBound.x < b.upperBound.x)
	{
		a->upperBound.x = b.upperBound.x;
		changed = true;
	}

	if (a->upperBound.y < b.upperBound.y)
	{
		a->upperBound.y = b.upperBound.y;
		changed = true;
	}

	return changed;
}

/// Does a fully contain b
static inline bool b2AABB_Contains(b2AABB a, b2AABB b)
{
	bool s = true;
	s = s && a.lowerBound.x <= b.lowerBound.x;
	s = s && a.lowerBound.y <= b.lowerBound.y;
	s = s && b.upperBound.x <= a.upperBound.x;
	s = s && b.upperBound.y <= a.upperBound.y;
	return s;
}

static inline bool b2AABB_ContainsWithMargin(b2AABB a, b2AABB b, float margin)
{
	bool s = (a.lowerBound.x <= b.lowerBound.x - margin) & (a.lowerBound.y <= b.lowerBound.y - margin) &
			 (b.upperBound.x + margin <= a.upperBound.x) & (b.upperBound.y + margin <= a.upperBound.y);
	return s;
}

/// Do a and b overlap
static inline bool b2AABB_Overlaps(b2AABB a, b2AABB b)
{
	b2Vec2 d1 = {b.lowerBound.x - a.upperBound.x, b.lowerBound.y - a.upperBound.y};
	b2Vec2 d2 = {a.lowerBound.x - b.upperBound.x, a.lowerBound.y - b.upperBound.y};

	if (d1.x > 0.0f || d1.y > 0.0f)
		return false;

	if (d2.x > 0.0f || d2.y > 0.0f)
		return false;

	return true;
}

#ifdef __cplusplus
}
#endif
