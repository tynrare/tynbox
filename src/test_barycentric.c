#include "./tests.h"

Vector3 get_barecentric_coordinates(Vector3 v0, Vector3 v1, Vector3 v2,
                                           Vector3 p) {
  Vector3 v0v1 = Vector3Subtract(v1, v0);
  Vector3 v0v2 = Vector3Subtract(v2, v0);
  Vector3 p_v0 = Vector3Subtract(p, v0);

  float d00 = Vector3DotProduct(v0v1, v0v1);
  float d01 = Vector3DotProduct(v0v1, v0v2);
  float d11 = Vector3DotProduct(v0v2, v0v2);
  float d20 = Vector3DotProduct(p_v0, v0v1);
  float d21 = Vector3DotProduct(p_v0, v0v2);

  float denom = d00 * d11 - d01 * d01;

  float v = (d11 * d20 - d01 * d21) / denom;
  float w = (d00 * d21 - d01 * d20) / denom;
  float u = 1.0 - v - w;

  Vector3 pos = {u, v, w};

  return pos;
}

Vector3 barycentric_to_cartesian(Vector3 bc_pos, Vector3 v0, Vector3 v1,
                                        Vector3 v2) {
  Vector3 a = Vector3Scale(v0, bc_pos.x);
  Vector3 b = Vector3Scale(v1, bc_pos.y);
  Vector3 c = Vector3Scale(v2, bc_pos.z);

  return Vector3Add(Vector3Add(a, b), c);
}

void _draw_test_barycentric() {
  Vector3 p1 = {-1, 0, -1};
  Vector3 p2 = {-1, 0, 1};
  Vector3 p3 = {1, 0, 0};
  Vector2 mp2d = GetMousePosition();
  Vector3 mp = {mp2d.x, 0, mp2d.y};

  Camera3D camera = {0};
  camera.position = (Vector3){0.0f, 10.0f, 0.01f};
  camera.target = (Vector3){0.0f, 0.0f, 0.0f};
  camera.up = (Vector3){0.0f, 1.0f, 0.0f};
  camera.fovy = 45.0f;
  camera.projection = CAMERA_PERSPECTIVE;

  Ray ray = GetScreenToWorldRay(GetMousePosition(), camera);
  Vector3 g0 = (Vector3){-50.0f, 0.0f, -50.0f};
  Vector3 g1 = (Vector3){-50.0f, 0.0f, 50.0f};
  Vector3 g2 = (Vector3){50.0f, 0.0f, 50.0f};
  Vector3 g3 = (Vector3){50.0f, 0.0f, -50.0f};
  RayCollision plane_hit_info = GetRayCollisionQuad(ray, g0, g1, g2, g3);

  Vector3 bc_pos =
      get_barecentric_coordinates(p1, p2, p3, plane_hit_info.point);
  Vector3 bc_pos_clamped = {bc_pos.x, bc_pos.y, bc_pos.z};
  bc_pos_clamped.x = fminf(fmaxf(bc_pos_clamped.x, 0), 1);
  bc_pos_clamped.y = fminf(fmaxf(bc_pos_clamped.y, 0), 1);
  bc_pos_clamped.z = fminf(fmaxf(bc_pos_clamped.z, 0), 1);
  float bc_sum = bc_pos_clamped.x + bc_pos_clamped.y + bc_pos_clamped.z;
  bc_pos_clamped.x /= bc_sum;
  bc_pos_clamped.y /= bc_sum;
  bc_pos_clamped.z /= bc_sum;
  Vector3 back_from_bc = barycentric_to_cartesian(bc_pos_clamped, p1, p2, p3);

  BeginMode3D(camera);
  DrawSphere(back_from_bc, 0.2, BLUE);
  DrawSphere(plane_hit_info.point, 0.1, RED);
  DrawTriangle3D(p1, p2, p3, WHITE);
  EndMode3D();

  Vector2 p12d = GetWorldToScreen(p1, camera);
  Vector2 p22d = GetWorldToScreen(p2, camera);
  Vector2 p32d = GetWorldToScreen(p3, camera);
  DrawText("p1", p12d.x, p12d.y, 20, RED);
  DrawText("p2", p22d.x, p22d.y, 20, RED);
  DrawText("p3", p32d.x, p32d.y, 20, RED);

  DrawText(TextFormat("%.2f(%.2f),%.2f(%.2f),%.2f(%.2f)", bc_pos.x,
                      bc_pos_clamped.x, bc_pos.y, bc_pos_clamped.y, bc_pos.z,
                      bc_pos_clamped.z),
           16, 40, 20, WHITE);
}
