#include <stdbool.h>
#include "tynroar_lib.h"

#define CONSOLE_HEIGHT 10
#define CONSOLE_LINE_HEIGHT 20
#define CONSOLE_MAX_CHARS 20
#define CONSOLE_OUTPUT_MAX_CHARS 200

#ifndef TYNROAR_RCMD_H
#define TYNROAR_RCMD_H

typedef struct ConsoleState {
  char inputstring[CONSOLE_MAX_CHARS + 1];
  char inputcommand[CONSOLE_MAX_CHARS + 1];
  char outputstring[CONSOLE_OUTPUT_MAX_CHARS + 1];
  int inputlength;
  bool inputonce;
  bool ready; // check for command confirmation
} ConsoleState;


STAGEFLAG Console_UpdateLogicFrame(ConsoleState *state, STAGEFLAG flags);
void Console_UpdateDrawFrame(ConsoleState *state);
void Console_Dispose(ConsoleState *state);
void Console_Init(TynStage *stage);
char *Console_ConfirmCommand(ConsoleState *state, STAGEFLAG *flags);
void Console_Print(ConsoleState *state, char *message);

#endif // !TYNROAR_RCMD_H
