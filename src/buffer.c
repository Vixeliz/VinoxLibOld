#define EGL_NO_X11
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include "buffer.h"

static PFNGLGENVERTEXARRAYSOESPROC glGenVertexArraysOES;
static PFNGLBINDVERTEXARRAYOESPROC glBindVertexArrayOES;
static PFNGLDELETEVERTEXARRAYSOESPROC glDeleteVertexArraysOES;

int vinoxCreateBuffer(Buffer *buffer) {
    
    /* Load extensions */
    glGenVertexArraysOES = 
    (PFNGLGENVERTEXARRAYSOESPROC)
    eglGetProcAddress("glGenVertexArraysOES");
    
    glBindVertexArrayOES = 
    (PFNGLBINDVERTEXARRAYOESPROC)
    eglGetProcAddress("glBindVertexArrayOES");
    
    glDeleteVertexArraysOES =
    (PFNGLDELETEVERTEXARRAYSOESPROC)
    eglGetProcAddress("glDeleteVertexArraysOES");

    glGenVertexArraysOES(1, &buffer->vao);

    glGenBuffers(1, &buffer->vbo);

    glGenBuffers(1, &buffer->ebo);
    glBindVertexArrayOES(buffer->vao);

    /* This preallocates our max indices for every draw call */
    uint32_t indices[INDICESCOUNT];
    uint32_t offset = 0;
    
    for (size_t i = 0; i < INDICESCOUNT; i += 6) {
        indices[i + 0] = 0 + offset;
        indices[i + 1] = 1 + offset;
        indices[i + 2] = 2 + offset;
        
        indices[i + 3] = 2 + offset;
        indices[i + 4] = 3 + offset;
        indices[i + 5] = 0 + offset;
        
        offset += 4;
    }
    
    /* This binds an area of memory so we can later change that area of memory
     * dynamically */
    glBindBuffer(GL_ARRAY_BUFFER, buffer->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * MAXVERTEXCOUNT, 
            NULL, GL_DYNAMIC_DRAW);
    
    /* Indices don't need to have there data changed since they follow a pattern
     * so we just set it to a static buffer */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    /* Tell our buffers where to look in the vertices for different types of
     * data. Since we do everything dynamically we store data in the vertices
     * including color, texcoords, texindex, and position */
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
            (void*)offsetof(Vertex, position));
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
            (void*)offsetof(Vertex, color));
    
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
            (void*)offsetof(Vertex, texCoords));
    
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex),
            (void*)offsetof(Vertex, texIndex));

    return 0;

}

/* Originally was in with create framebuffer but we moved it to resize so we can
 * change the width and height of the texture based off of the window */
int vinoxResizeFramebuffer(FrameBuffer *frameBuffer) {
    glActiveTexture(frameBuffer->texture.id);
    glBindTexture(GL_TEXTURE_2D, frameBuffer->texture.id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frameBuffer->texture.width, frameBuffer->texture.height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, frameBuffer->scaleType);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, frameBuffer->scaleType);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frameBuffer->texture.id, 0);
    return 0;
}

int vinoxCreateFramebuffer(FrameBuffer *frameBuffer) {
    
    /* Framebuffer buffers */
    float vertices[] = {
        -1.0f, 1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 0.0f,

        -1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f
    };
    
    /* Same story gen buffers this time a static buffer since we don't need to
     * change the vertices dynamically */
    glGenVertexArraysOES(1, &frameBuffer->vao);
    glGenBuffers(1, &frameBuffer->vbo);
    glBindVertexArrayOES(frameBuffer->vao);
    glBindBuffer(GL_ARRAY_BUFFER, frameBuffer->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
    
    /* Yet again where to look in the vertices for data left being positions
     * right being texture coords */
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, 
            (void*)(2 * sizeof(float)));

    /* This generates the actual framebuffer itself and binds it to the current
     * one we then generate a texture for the resize function to edit */
    glGenFramebuffers(1, &frameBuffer->fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer->fbo);
    
    glGenTextures(1, &frameBuffer->texture.id);
    vinoxResizeFramebuffer(frameBuffer);

    /* Make sure it was actually created */
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        printf("Framebuffer failure!\n");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArrayOES(0);
    return 0;
}
