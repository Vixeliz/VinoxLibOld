#ifndef BUFFER_H
#define BUFFER_H

#include "vingl.h"
#include "texture.h"

#define MAXQUADCOUNT 25000
#define MAXVERTEXCOUNT MAXQUADCOUNT * 4
#define INDICESCOUNT MAXQUADCOUNT * 6

typedef struct {
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    Vertex vertices[MAXVERTEXCOUNT];
} Buffer;

int vinoxCreateBuffer(Buffer *buffer);
int vinoxCreateFramebuffer(FrameBuffer *frameBuffer);
int vinoxResizeFramebuffer(FrameBuffer *frameBuffer);
#endif
