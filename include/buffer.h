#ifndef BUFFER_H
#define BUFFER_H

#include "vingl.h"

#define MAXQUADCOUNT 8000
#define MAXVERTEXCOUNT MAXQUADCOUNT * 4
#define INDICESCOUNT MAXQUADCOUNT * 6

typedef struct {
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    Vertex vertices[MAXVERTEXCOUNT];
} Buffer;

typedef struct {
    GLuint vao;
    GLuint vbo;
    GLuint fbo;
    GLuint textureColorbuffer;
} FrameBuffer;

int vinoxCreateBuffer(Buffer *buffer);
int vinoxCreateFramebuffer(FrameBuffer *frameBuffer, int width, int height);
int vinoxResizeFramebuffer(FrameBuffer *frameBuffer, int width, int height);
#endif
