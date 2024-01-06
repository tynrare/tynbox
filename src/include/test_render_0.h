#include "tynroar_lib.h"
#include "raylib.h"

#ifndef TEST_RENDER0_H
#define TEST_RENDER0_H

typedef struct {
	Model model;
	Texture2D texture;
	Camera camera;
	Shader shader;
	RenderTexture2D tex_target;
} TestRender0State;

TestRender0State* TestRender0Init(TynStage* stage);

#endif