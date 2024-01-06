#include "include/tyncommons.h"
#include <stdarg.h>
#include "raylib.h"
#include <math.h>

bool isAnyKeyPressed(int count, ...) {
  bool pressed = false;

  va_list args;
  va_start(args, count);

  for (int i = 0; i < count; i++) {
    if (IsKeyDown(va_arg(args, int))) {
      pressed = true;
    }
  }

  va_end(args);

  return pressed;
}

int min(int a, int b) { return a > b ? b : a; }

float lerp(float a, float b, float t) { return a + (b - a) * t; }

float rlerp(float a, float b, float t) {
  float CS = (1.0f - t) * cosf(a) + t * cosf(b);
  float SN = (1.0f - t) * sinf(a) + t * sinf(b);

  return atan2f(SN, CS);
}