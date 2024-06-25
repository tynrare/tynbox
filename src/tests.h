#include <raylib.h>
#include <raymath.h>

// --- barycentric

Vector3 get_barecentric_coordinates(Vector3 v0, Vector3 v1, Vector3 v2,
                                    Vector3 p);

Vector3 barycentric_to_cartesian(Vector3 bc_pos, Vector3 v0, Vector3 v1,
                                 Vector3 v2);
void _draw_test_barycentric();

// --- lerp

void _draw_test_lerp();
