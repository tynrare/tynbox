#include "include/shader_sdf2d.h"
#include <math.h>

const int Raymarch2dDatasetSize = 2048;
const char* Raymarch2dfragShaderFileName = "res/shaders/tynroar-sdf.fs";

Color Raymarch2dPosToColor(int x, int y) {
    return (Color) {
        (unsigned char)x, (unsigned char)((int)x >> 8),
            (unsigned char)y, (unsigned char)((int)y >> 8)
    };
}

Vector2 Raymarch2dColorToPos(Color c) {
    return (Vector2) { c.r | (c.g << 8), c.b | (c.a << 8) };
}

Vector2 Raymarch2dIndexToPos(int index, int xdimension) {
    return (Vector2) { index% xdimension, floor((float)index / (float)xdimension) };
}

/**
 * @returns {int} new empty index
 */
int Raymarch2dWriteEntity(int shift, Raymarch2dShapeType type, Vector2 position, Vector2 size,
    int rotation) {
    DrawPixelV(Raymarch2dIndexToPos(shift, Raymarch2dDatasetSize), (Color) { 4, 0, 0, 0 });

    // entity type
    // a == 0 : circle
    // a == 1 : box
    DrawPixelV(Raymarch2dIndexToPos(shift + 1, Raymarch2dDatasetSize), (Color) { (int)type, 0, 0, 0 });
    // entity position
    DrawPixelV(Raymarch2dIndexToPos(shift + 2, Raymarch2dDatasetSize),
        Raymarch2dPosToColor((int)position.x, (int)position.y));
    // entity size
    DrawPixelV(Raymarch2dIndexToPos(shift + 3, Raymarch2dDatasetSize), Raymarch2dPosToColor(size.x, size.y));
    // entity rotation
    DrawPixelV(Raymarch2dIndexToPos(shift + 4, Raymarch2dDatasetSize), Raymarch2dPosToColor(rotation, 0));

    return shift + 5;
}

void Raymarch2dInit(RenderTexture2D* texdataset, Shader* shader) {
    // does not for for indices > Raymarch2dDatasetSize 
    *texdataset = LoadRenderTexture(Raymarch2dDatasetSize, 2);

    // Load raymarching shader
    // NOTE: Defining 0 (NULL) for vertex shader forces usage of internal default
    // vertex shader
    *shader = LoadShader(0, Raymarch2dfragShaderFileName);

    // Get shader locations for required uniforms
    int resolutionLoc = GetShaderLocation(*shader, "resolution");

    float resolution[2] = { (float)GetScreenWidth(), (float)GetScreenHeight() };
    SetShaderValue(*shader, resolutionLoc, resolution, SHADER_UNIFORM_VEC2);
}

void Raymarch2dDraw(RenderTexture2D* texdataset, Shader* shader) {
    int datasetLoc = GetShaderLocation(*shader, "dataset");

    BeginShaderMode(*shader);
    SetShaderValueTexture(*shader, datasetLoc, texdataset->texture);
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), WHITE);
    EndShaderMode();
}

void Raymarch2dBeginWrite(RenderTexture2D* texdataset, bool clear) {
    BeginTextureMode(*texdataset);
    BeginBlendMode(BLEND_ADD_COLORS);
    if (clear) {
        ClearBackground(BLANK);
    }
}
void Raymarch2dEndWrite() {
    EndBlendMode();
    EndTextureMode();
}
