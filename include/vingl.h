#ifndef VINGL_H
#define VINGL_H

#include "raymath.h"
#include "camera.h"

typedef struct {
    Vector3 position;
    Vector4 color;
    Vector2 texCoords;
    float texIndex;
} Vertex;

int vinoxInit(int width, int height);
void vinoxBeginDrawing(Camera camera, int width, int height);
void vinoxEndDrawing();
int vinoxEnd();

unsigned int vinoxCreateTexture(const char* path);
int vinoxCreateQuad(float x, float y, float width, float height, float textureID, Vector4 color);

#endif
