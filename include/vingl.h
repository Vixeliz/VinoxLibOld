#ifndef VINGL_H
#define VINGL_H

#include "raymath.h"
#include "camera.h"
#include "texture.h"

typedef struct {
    Vector3 position;
    Vector4 color;
    Vector2 texCoords;
    float texIndex;
} Vertex;

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
int vinoxCreateQuad(float x, float y, float width, float height, float textureID, Vector4 color);

#endif
