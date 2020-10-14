#ifndef VINGL_H
#define VINGL_H

#include "raymath.h"
#include "buffer.h"
#include "camera.h"

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
int vinoxCreateQuad(Quad quad, Quad textureMask, float textureID, Vector4 color, float rotation);

#endif
