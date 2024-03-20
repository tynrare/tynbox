// original version:
// https://github.com/raysan5/raylib/blob/master/examples/shaders/shaders_deferred_render.c

#include "include/deferred_render.h"
#include "rlgl.h"
#include <stdlib.h>

#define RLIGHTS_IMPLEMENTATION
#include "external/rlights.h"

void LoadDeferredRender(DeferredRenderState *state, int width, int height) {
  state->width = width;
  state->height = width;
  state->render_target = LoadRenderTexture(width, height);
  // Load geometry buffer (G-buffer) shader and deferred shader
  state->gbufferShader =
      LoadShader("res/shaders/gbuffer.vs", "res/shaders/gbuffer.fs");

  state->deferredShader = LoadShader("res/shaders/deferred_shading.vs",
                                     "res/shaders/deferred_shading.fs");
  state->deferredShader.locs[SHADER_LOC_VECTOR_VIEW] =
      GetShaderLocation(state->deferredShader, "viewPosition");

  // Initialize the G-buffer
  state->gbuffer.framebuffer = rlLoadFramebuffer();

  if (!state->gbuffer.framebuffer) {
    TraceLog(LOG_WARNING, "Failed to create framebuffer");
    return;
  }

  rlEnableFramebuffer(state->gbuffer.framebuffer);

  // Since we are storing position and normal data in these textures,
  // we need to use a floating point format.
  state->gbuffer.positionTexture = rlLoadTexture(
      NULL, width, height, RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32, 1);

  state->gbuffer.normalTexture = rlLoadTexture(
      NULL, width, height, RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32, 1);
  // Albedo (diffuse color) and specular strength can be combined into one
  // texture. The color in RGB, and the specular strength in the alpha channel.
  state->gbuffer.albedoSpecTexture = rlLoadTexture(
      NULL, width, height, RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, 1);

  // Activate the draw buffers for our framebuffer
  rlActiveDrawBuffers(3);

  // Now we attach our textures to the framebuffer.
  rlFramebufferAttach(state->gbuffer.framebuffer,
                      state->gbuffer.positionTexture,
                      RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_TEXTURE2D, 0);
  rlFramebufferAttach(state->gbuffer.framebuffer, state->gbuffer.normalTexture,
                      RL_ATTACHMENT_COLOR_CHANNEL1, RL_ATTACHMENT_TEXTURE2D, 0);
  rlFramebufferAttach(state->gbuffer.framebuffer,
                      state->gbuffer.albedoSpecTexture,
                      RL_ATTACHMENT_COLOR_CHANNEL2, RL_ATTACHMENT_TEXTURE2D, 0);

  // Finally we attach the depth buffer.
  state->gbuffer.depthRenderbuffer = rlLoadTextureDepth(width, height, true);
  rlFramebufferAttach(state->gbuffer.framebuffer,
                      state->gbuffer.depthRenderbuffer, RL_ATTACHMENT_DEPTH,
                      RL_ATTACHMENT_RENDERBUFFER, 0);

  // Make sure our framebuffer is complete.
  // NOTE: rlFramebufferComplete() automatically unbinds the framebuffer, so we
  // don't have to rlDisableFramebuffer() here.
  if (!rlFramebufferComplete(state->gbuffer.framebuffer)) {
    TraceLog(LOG_WARNING, "Framebuffer is not complete");
    return;
  }

  // Now we initialize the sampler2D uniform's in the deferred shader.
  // We do this by setting the uniform's value to the color channel slot we
  // earlier bound our textures to.
  rlEnableShader(state->deferredShader.id);

  rlSetUniformSampler(
      rlGetLocationUniform(state->deferredShader.id, "gPosition"), 0);
  rlSetUniformSampler(rlGetLocationUniform(state->deferredShader.id, "gNormal"),
                      1);
  rlSetUniformSampler(
      rlGetLocationUniform(state->deferredShader.id, "gAlbedoSpec"), 2);

  rlDisableShader();

  rlEnableDepthTest();
}

void UnloadDeferredRender(DeferredRenderState *state) {
  UnloadShader(state->deferredShader); // Unload shaders
  UnloadShader(state->gbufferShader);

  // Unload geometry buffer and all attached textures
  rlUnloadFramebuffer(state->gbuffer.framebuffer);
  rlUnloadTexture(state->gbuffer.positionTexture);
  rlUnloadTexture(state->gbuffer.normalTexture);
  rlUnloadTexture(state->gbuffer.albedoSpecTexture);
  rlUnloadTexture(state->gbuffer.depthRenderbuffer);
}

void BeginDrawDeferredRender(DeferredRenderState *state, Camera camera) {
  BeginTextureMode(state->render_target);
  float cameraPos[3] = {camera.position.x, camera.position.y,
                        camera.position.z};
  SetShaderValue(state->deferredShader,
                 state->deferredShader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos,
                 SHADER_UNIFORM_VEC3);

  // Draw to the geometry buffer by first activating it
  rlEnableFramebuffer(state->gbuffer.framebuffer);
  rlClearScreenBuffers(); // Clear color and depth buffer

  rlDisableColorBlend();
  BeginMode3D(camera);
  // NOTE: We have to use rlEnableShader here. `BeginShaderMode` or thus
  // `rlSetShader` will not work, as they won't immediately load the shader
  // program.
  rlEnableShader(state->gbufferShader.id);

  // --->>
}

// some render code should be called inbetween

void EndDrawDeferredRender(DeferredRenderState *state, Camera camera, int width,
                           int height, DeferredRenderMode mode) {
  // <<---

  rlDisableShader();
  EndMode3D();
  rlEnableColorBlend();

  // Go back to the default framebuffer (0) and draw our deferred shading.
  rlDisableFramebuffer();
  rlClearScreenBuffers(); // Clear color & depth buffer

  // tryin to make proper resize
  float w = width;
  float h = height;
  float ratio = h / w;
  Rectangle source = (Rectangle){0, 0, state->width, -state->height};
  Rectangle dest = (Rectangle){(w - w * ratio) / 2, 0, w * ratio, h};

  switch (mode) {
  case DEFERRED_SHADING: {
    BeginMode3D(camera);
    rlDisableColorBlend();
    rlEnableShader(state->deferredShader.id);
    // Activate our g-buffer textures
    // These will now be bound to the sampler2D uniforms `gPosition`, `gNormal`,
    // and `gAlbedoSpec`
    rlActiveTextureSlot(0);
    rlEnableTexture(state->gbuffer.positionTexture);
    rlActiveTextureSlot(1);
    rlEnableTexture(state->gbuffer.normalTexture);
    rlActiveTextureSlot(2);
    rlEnableTexture(state->gbuffer.albedoSpecTexture);

    // Finally, we draw a fullscreen quad to our default framebuffer
    // This will now be shaded using our deferred shader
    rlLoadDrawQuad();
    rlDisableShader();
    rlEnableColorBlend();
    EndMode3D();

    // As a last step, we now copy over the depth buffer from our g-buffer to
    // the default framebuffer.
    rlBindFramebuffer(RL_READ_FRAMEBUFFER, state->gbuffer.framebuffer);
    rlBindFramebuffer(RL_DRAW_FRAMEBUFFER, 0);
    rlBlitFramebuffer(0, 0, state->width, state->height, 0, 0, state->width, state->height,
                      0x00000100); // GL_DEPTH_BUFFER_BIT
    rlDisableFramebuffer();

		EndTextureMode();
    DrawTexturePro(state->render_target.texture, source, dest, (Vector2){0, 0}, 0, WHITE);

    /*
BeginMode3D(camera);
rlEnableShader(rlGetShaderIdDefault());
    // ... some additional draw possible here
rlDisableShader();
EndMode3D();
    */

    // DrawText("FINAL RESULT", 10, height - 30, 20, DARKGREEN);
  } break;
  case DEFERRED_POSITION: {
		EndTextureMode();
    Texture2D texture = (Texture2D){
        .id = state->gbuffer.positionTexture,
        .width = state->width,
        .height = state->height,
    };
    DrawTexturePro(texture, source, dest, (Vector2){0, 0}, 0, WHITE);

    // DrawText("POSITION TEXTURE", 10, height - 30, 20, DARKGREEN);
  } break;
  case DEFERRED_NORMAL: {
		EndTextureMode();
    Texture2D texture = (Texture2D){
        .id = state->gbuffer.normalTexture,
        .width = state->width,
        .height = state->height,
    };
    DrawTexturePro(texture, source, dest, (Vector2){0, 0}, 0, WHITE);

    // DrawText("NORMAL TEXTURE", 10, height - 30, 20, DARKGREEN);
  } break;
  case DEFERRED_ALBEDO: {
		EndTextureMode();
    Texture2D texture = (Texture2D){
        .id = state->gbuffer.albedoSpecTexture,
        .width = state->width,
        .height = state->height,
    };
    DrawTexturePro(texture, source, dest, (Vector2){0, 0}, 0, WHITE);

    // DrawText("ALBEDO TEXTURE", 10, height - 30, 20, DARKGREEN);
  } break;
  default:
    break;
  }
}
