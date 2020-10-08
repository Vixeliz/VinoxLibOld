#define EGL_NO_X11
#include <EGL/egl.h>
#define GL_GLEXT_PROTOTYPES 1
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include "buffer.h"

int vinoxCreateBuffer(Buffer *buffer) {

    /* Bind VAO extension */
    PFNGLGENVERTEXARRAYSOESPROC glGenVertexArraysOES;
    PFNGLBINDVERTEXARRAYOESPROC glBindVertexArrayOES;
    PFNGLDELETEVERTEXARRAYSOESPROC glDeleteVertexArraysOES;
    PFNGLISVERTEXARRAYOESPROC glIsVertexArrayOES;
    glGenVertexArraysOES = 
    (PFNGLGENVERTEXARRAYSOESPROC)
    eglGetProcAddress("glGenVertexArraysOES");
    
    glBindVertexArrayOES = 
    (PFNGLBINDVERTEXARRAYOESPROC)
    eglGetProcAddress("glBindVertexArrayOES");
    
    glDeleteVertexArraysOES =
    (PFNGLDELETEVERTEXARRAYSOESPROC)
    eglGetProcAddress("glDeleteVertexArraysOES");
    
    glIsVertexArrayOES = 
    (PFNGLISVERTEXARRAYOESPROC)
    eglGetProcAddress("glIsVertexArrayOES");
    
    glGenVertexArraysOES(1, &buffer->vao);

    glGenBuffers(1, &buffer->vbo);

    GLuint ebo;
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
    
    glBindBuffer(GL_ARRAY_BUFFER, buffer->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * MAXVERTEXCOUNT, 
            NULL, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
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

int vinoxResizeFramebuffer(FrameBuffer *frameBuffer, int width, int height) {
    //glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_2D, frameBuffer->textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frameBuffer->textureColorbuffer, 0);
    //glActiveTexture(GL_TEXTURE0 + 1);
    return 0;
}

int vinoxCreateFramebuffer(FrameBuffer *frameBuffer, int width, int height) {
    
    PFNGLGENVERTEXARRAYSOESPROC glGenVertexArraysOES;
    PFNGLBINDVERTEXARRAYOESPROC glBindVertexArrayOES;
    
    glGenVertexArraysOES = 
    (PFNGLGENVERTEXARRAYSOESPROC)
    eglGetProcAddress("glGenVertexArraysOES");
    
    glBindVertexArrayOES = 
    (PFNGLBINDVERTEXARRAYOESPROC)
    eglGetProcAddress("glBindVertexArrayOES");
    
    /* Framebuffer buffers */
    float vertices[] = {
        -1.0f, 1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 0.0f,

        -1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f
    };
    
    glGenVertexArraysOES(1, &frameBuffer->vao);
    glGenBuffers(1, &frameBuffer->vbo);
    glBindVertexArrayOES(frameBuffer->vao);
    glBindBuffer(GL_ARRAY_BUFFER, frameBuffer->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, 
            (void*)(2 * sizeof(float)));

    glGenFramebuffers(1, &frameBuffer->fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer->fbo);
    
    glGenTextures(1, &frameBuffer->textureColorbuffer);

    vinoxResizeFramebuffer(frameBuffer, width, height);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        printf("Framebuffer failure!\n");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return 0;
}


