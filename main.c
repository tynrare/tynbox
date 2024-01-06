#include "raylib.h"
#include "raymath.h"
#include "src/include/app.h"
#include <stdio.h>
#include <stdlib.h>

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
int screenWidth = 512;
int screenHeight = 512;

//----------------------------------------------------------------------------------
// Main Enry Point
//----------------------------------------------------------------------------------
int main() {
  // Initialization
  //--------------------------------------------------------------------------------------

    // SetConfigFlags(FLAG_MSAA_4X_HINT); // Set MSAA 4X hint before windows
    //  creation

  InitWindow(screenWidth, screenHeight, "tynbox 240106");

  TynStage stage = {0};
  AppState *state = AppInit(&stage);

  // DisableCursor();

#if defined(PLATFORM_WEB)
  emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
  throw "unimplemented"
#else
#endif
  SetTargetFPS(60); 

  while (!WindowShouldClose()) // Detect window close button or ESC key
  {
    STAGEFLAG flags = stage.frame.step(state, stage.flags);

    if (flags & STAGEFLAG_DISABLED) {
      break;
    }

    BeginDrawing();
    
    stage.frame.draw(state);
    
    EndDrawing();
  }

  stage.frame.dispose(state);

  return 0;
}
