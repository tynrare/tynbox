#include <raylib.h>

#ifndef DEFERRED_RENDER_H
#define DEFERRED_RENDER_H

typedef struct DeferredRenderGBuffer {
    unsigned int framebuffer;

    unsigned int positionTexture;
    unsigned int normalTexture;
    unsigned int albedoSpecTexture;
    
    unsigned int depthRenderbuffer;
} DeferredRenderGBuffer;

typedef struct DeferredRenderState {
	DeferredRenderGBuffer gbuffer;
	Shader gbufferShader;
	Shader deferredShader;

	// attemptin to make proper texture scaling
	// mostly just stores sizes and forces to draw framebuffer
	// by BeginTextureMode
	RenderTexture2D render_target;

	int width;
	int height;
} DeferredRenderState;

typedef enum {
   DEFERRED_POSITION,
   DEFERRED_NORMAL,
   DEFERRED_ALBEDO,
   DEFERRED_SHADING
} DeferredRenderMode;

void LoadDeferredRender(DeferredRenderState *state, int width, int height);
void UnloadDeferredRender(DeferredRenderState *state);
void BeginDrawDeferredRender(DeferredRenderState *state, Camera camera);
void EndDrawDeferredRender(DeferredRenderState *state, Camera camera, int width, int height, DeferredRenderMode mode);

#endif
