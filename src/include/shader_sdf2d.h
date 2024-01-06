#include "raylib.h"

#ifndef RAYMARCH2D_H
#define RAYMARCH2D_H

typedef enum { CIRCLE = 0, BOX = 1 } Raymarch2dShapeType;

Color Raymarch2dPosToColor(int x, int y);
Vector2 Raymarch2dColorToPos(Color c);
Vector2 Raymarch2dIndexToPos(int index, int xdimension);
int Raymarch2dWriteEntity(int shift, Raymarch2dShapeType type, Vector2 position, Vector2 size,
    int rotation);
void Raymarch2dInit(RenderTexture2D* texdataset, Shader* shader);
void Raymarch2dDraw(RenderTexture2D* texdataset, Shader* shader);
void Raymarch2dBeginWrite(RenderTexture2D* texdataset, bool clear);
void Raymarch2dEndWrite();

extern const char* Raymarch2dfragShaderFileName;
extern const int Raymarch2dDatasetSize;

#endif