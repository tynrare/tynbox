#include "tynroar_rcmd.h"
#include "tynroar_lib.h"
#include <stdbool.h>

#ifndef APP_H
#define APP_H

#define APP_STAGES_TOTAL 8

typedef enum { APPCODE_DEFAULT, APPCODE_TERMINATE } APPCODE;


typedef struct AppState {
  int activestages;
  TynStage *stages[APP_STAGES_TOTAL];
} AppState;

AppState *AppInit(TynStage *stage);

#endif