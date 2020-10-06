#ifndef VINGL_H
#define VINGL_H

#include <cglm/cglm.h>

typedef struct {
    vec3 position;
    vec4 color;
    vec2 texCoords;
    float texIndex;
} Vertex;

typedef struct {
    float rotation;
    float scale;
    vec2 position;
    vec2 origin;
} Camera;

int vinoxInit();
void vinoxBeginDrawing(Camera camera, int width, int height);
void vinoxEndDrawing();
int vinoxEnd();

unsigned int vinoxCreateTexture(const char* path);
int vinoxCreateQuad(float x, float y, float width, float height, float textureID, vec4 color);

#endif
