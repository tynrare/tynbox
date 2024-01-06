#include "tynroar_lib.h"
#include "raylib.h"

#ifndef TEST_SHADER0_H
#define TEST_SHADER0_H

typedef struct {
  RenderTexture2D texdataset;
  Shader shader;
  float totalTime;
  long int fragShaderFileModTime;
  bool shaderAutoReloading;
} TestShader0State;

TestShader0State *TestShader0Init(TynStage *stage);

#endif