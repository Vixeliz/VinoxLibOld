#ifndef TEXTURE_H
#define TEXTURE_H

#include <GLES2/gl2.h>

typedef struct {
    unsigned int id;
    int width, height;
} Texture;

typedef struct {
    GLuint vao;
    GLuint vbo;
    GLuint fbo;
    GLint scaleType;
    Texture texture;
} FrameBuffer;


int vinoxLoadTexture(const char* filepath, Texture *texture, GLint scaleType);

#endif
