#ifndef VINGL_H
#define VINGL_H

#include <cglm/cglm.h>

typedef struct {
    float x, y;
} Vec2;
typedef struct {
    float x, y, z;
}Vec3;
typedef struct {
    float x, y, z, w;
}Vec4;

typedef struct {
    Vec3 position;
    Vec4 color;
    Vec2 texCoords;
    float texIndex;
} Vertex;

typedef struct {
    float rotation;
    float scale;
    Vec2 position;
    Vec2 origin;
} Camera;

int vinoxInit();
void vinoxBeginDrawing(Camera camera, int width, int height);
void vinoxEndDrawing();
int vinoxEnd();

unsigned int vinoxCreateTexture(const char* path);
Vertex* vinoxCreateQuad(Vertex* target, float x, float y, float width, float height, 
        float textureID, Vec4 color);

#endif
