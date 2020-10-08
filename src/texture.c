#define EGL_NO_X11
#include <EGL/egl.h>
#define GL_GLEXT_PROTOTYPES 1
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include "texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int vinoxLoadTexture(const char* path, Texture *texture) {
    int c;
    /* Load the texture we should error check here */
    unsigned char *data = stbi_load(path, &texture->width, &texture->height, &c, 0);
    /* Generate the GLtexture and free the texture after we are done */
    glGenTextures(1, &texture->id);
    glBindTexture(GL_TEXTURE_2D, texture->id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture->width, texture->height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    if(data == NULL)
        printf("Texture(%i) failed to load!\n", texture->id);
    
    stbi_image_free(data);
    
    return 0;
}
