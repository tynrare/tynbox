#include <string.h>
#include <raylib.h>

#include "include/tynroar_rcmd.h"
#include "include/app.h"
#include <stdlib.h>

STAGEFLAG Console_UpdateLogicFrame(ConsoleState *state, STAGEFLAG flags) {
	if (IsKeyPressed(KEY_TAB)) {
		flags ^= STAGEFLAG_DISABLEDDRAW;
		flags ^= STAGEFLAG_BLOCKSTEP;
	}
  
  if (flags & STAGEFLAG_DISABLEDDRAW) {
    return flags;
  }

  // Get char pressed (unicode character) on the queue
  int key = GetCharPressed();

  // Check if more characters have been pressed on the same frame
  while (key > 0) {
    // NOTE: Only allow keys in range [32..125]
    if ((key >= 32) && (key <= 125) && (state->inputlength < CONSOLE_MAX_CHARS)) {
      state->inputstring[state->inputlength] = (char)key;
      state->inputstring[state->inputlength + 1] =
          '\0'; // Add null terminator at the end of the string.
      state->inputlength++;
    }

    key = GetCharPressed(); // Check next character in the queue
  }

  if (IsKeyPressed(KEY_BACKSPACE)) {
    state->inputlength--;
    if (state->inputlength < 0)
      state->inputlength = 0;
    state->inputstring[state->inputlength] = '\0';
  }

  if (IsKeyPressed(KEY_ENTER)) {
    state->ready = true;
  }

  return flags;
}

char* Console_ConfirmCommand(ConsoleState* state, STAGEFLAG *flags) {
  if (!state->ready)
    return NULL;

  strcpy(state->inputcommand, state->inputstring);
  state->inputstring[0] = '\0';
  state->inputlength = 0;
  state->ready = false;

  *flags = (*flags) | CMDFLAG_BROADCAST;

  return state->inputcommand;
}

char *Console_Print(ConsoleState* state, char* message) {
  strcpy(state->outputstring, message);
}

void Console_UpdateDrawFrame(ConsoleState *state) {
  DrawRectangle(1, 1, GetScreenWidth() - 2,
                CONSOLE_LINE_HEIGHT * CONSOLE_HEIGHT, (Color){ 0, 0, 0, 100 });
  DrawRectangleLines(1, 1 + CONSOLE_LINE_HEIGHT * (CONSOLE_HEIGHT - 1),
                     GetScreenWidth() - 2, CONSOLE_LINE_HEIGHT, GREEN);
  DrawText(TextFormat("< %s", state->inputstring), 5,
           CONSOLE_LINE_HEIGHT * (CONSOLE_HEIGHT - 1) + 2, CONSOLE_LINE_HEIGHT,
           WHITE);
  DrawText(TextFormat("> %s\n", state->inputcommand), 5, 2, CONSOLE_LINE_HEIGHT,
           WHITE);
  DrawText(TextFormat("%s", state->outputstring), 10, CONSOLE_LINE_HEIGHT + 2,
           CONSOLE_LINE_HEIGHT,
           WHITE);
}

void Console_Init(TynStage *stage) {
  ConsoleState *state = malloc(sizeof(ConsoleState));
  state->inputonce = false;
  state->inputstring[0] = '\0';
  state->inputcommand[0] = '\0';
  state->outputstring[0] = '\0';
  state->inputlength = 0;
  state->ready = false;

  stage->state = state;
  stage->frame = (TynFrame){&Console_Dispose, &Console_UpdateLogicFrame,
                            &Console_UpdateDrawFrame, &Console_ConfirmCommand,
                            &Console_Print};

  stage->flags |= STAGEFLAG_DISABLEDDRAW;
}

void Console_Dispose(ConsoleState *state) { free(state); }
