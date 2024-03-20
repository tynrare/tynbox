#include "raylib.h"
#include "include/app.h"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
int screenWidth = 512;
int screenHeight = 512;

static TynStage stage = {0};
static AppState *state;
bool active = false;

void UpdateDrawFrame(void);     // Update and Draw one frame

//----------------------------------------------------------------------------------
// Main Enry Point
//----------------------------------------------------------------------------------
int main() {
  // Initialization
  //--------------------------------------------------------------------------------------

  //SetConfigFlags(FLAG_MSAA_4X_HINT); // Set MSAA 4X hint before windows
    //  creation

  InitWindow(512, 512, "tynbox 240310");

  state = AppInit(&stage);
  active = true;
  // DisableCursor();

#if defined(PLATFORM_WEB)
  emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
  SetTargetFPS(60); 

  while (!WindowShouldClose() && active) {
		UpdateDrawFrame();
	} // Detect window close button or ESC key
#endif

  stage.frame.dispose(state);
  CloseWindow();

  return 0;
}

void UpdateDrawFrame(void)
{
    STAGEFLAG flags = stage.frame.step(state, stage.flags);

    if (flags & STAGEFLAG_DISABLED) {
      active = false;
#if defined(PLATFORM_WEB)
      emscripten_cancel_main_loop();
#endif
      return;
    }

    BeginDrawing();
    
    stage.frame.draw(state);
    
    EndDrawing();
}
