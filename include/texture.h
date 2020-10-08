#ifndef TEXTURE_H
#define TEXTURE_H

typedef struct {
    unsigned int id;
    int width, height;
} Texture;

typedef struct {
    unsigned int id;
    Texture texture;
} RenderTexture;

int vinoxLoadTexture(const char* filepath, Texture *texture);

#endif
