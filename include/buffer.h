#ifndef BUFFER_H
#define BUFFER_H

#include "raymath.h"
#include "texture.h"

#define MAXQUADCOUNT 8192
#define MAXVERTEXCOUNT MAXQUADCOUNT * 4
#define INDICESCOUNT MAXQUADCOUNT * 6
#define MAXPOINTCOUNT MAXVERTEXCOUNT

typedef struct {
    Vector3 position;
    Vector4 color;
    Vector2 texCoords;
    float texIndex;
} Vertex;

typedef struct {
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    Vertex vertices[MAXVERTEXCOUNT];
} Buffer;

typedef struct {
    GLuint fbo;
    GLint scaleType;
    Texture texture;
} FrameBuffer;

int vinoxCreateQuadBuffer(Buffer *buffer);
int vinoxCreateFramebuffer(FrameBuffer *frameBuffer);
int vinoxResizeFramebuffer(FrameBuffer *frameBuffer);

#endif
