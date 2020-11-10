#ifndef VINGL_H
#define VINGL_H

#include "raymath.h"
#include "buffer.h"
#include "camera.h"

#define EMPTYQUAD (Quad) { { 0.0f, 0.0f }, { 0.0f, 0.0f } }
#define EMPTYVEC2 (Vector2) { 0.0f, 0.0f }

typedef struct {
    Vector2 position;
    Vector2 size;
} Quad;

int vinoxInit(int width, int height);
int vinoxBeginCamera(Camera *camera);
int vinoxEndCamera();
void vinoxBeginDrawing(int width, int height);
void vinoxEndDrawing();
int vinoxEnd();
int vinoxBeginTexture(FrameBuffer *frameBuffer);
int vinoxEndTexture(FrameBuffer *frameBuffer);
int vinoxClear(Vector4 color);

unsigned int vinoxCreateTexture(const char* path);
int vinoxQuad(Quad quad, Vector4 color);
int vinoxRotatedQuad(Quad quad, Vector4 color, float rotation);
int vinoxTexturedQuad(Quad quad, float textureID, Vector4 color);
int vinoxQuadPro(Quad quad, Quad textureMask, Vector2 textureSize, float textureID, Vector4 color, float rotation);

#endif
