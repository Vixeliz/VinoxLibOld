#define EGL_NO_X11
#include <EGL/egl.h>
#define GL_GLEXT_PROTOTYPES 1
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <stdio.h>
#include <string.h>
#include "vingl.h"
#include "camera.h"
#include "buffer.h"
#include "texture.h"
#include "shader.h"

/* Functions for the file */
static Vertex* createQuad(Vertex* target, float x, float y, float width, float height,
        float textureID, vec4 color);

/* Global variables in file */
typedef struct {
    Buffer buffer;
    FrameBuffer frameBuffer;
    ShaderProgram program;
    ShaderProgram screenProgram;
} vinState;

static ShaderProgram program;
static ShaderProgram screenProgram;
static vinState vinGLState = {0};
static uint32_t indexCount = 0;
static uint32_t vertexCount = 0;
static uint32_t drawCalls = 0;
static Vertex* buffer = vinGLState.buffer.vertices;
static int currentDrawCall = 0;
static int lastDrawCall = 0;

static int vinoxBeginTexture() {
    return 0;
}

static int vinoxEndTexture() {
    return 0;
}

static int drawBatch() {
    
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
    
    glBindBuffer(GL_ARRAY_BUFFER, vinGLState.buffer.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * MAXVERTEXCOUNT, &vinGLState.buffer.vertices[0]);
        
    glBindVertexArrayOES(vinGLState.buffer.vao);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);

    return 0;
}

int vinoxInit(int width, int height) {
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    program.type = 0;
    if (vinoxCompileShader(&program) == -1) {
        printf("Failed to compile world shader aborting\n");
        return -1;
    }

    screenProgram.type = 1;
    if (vinoxCompileShader(&screenProgram) == -1) {
        printf("Failed to compile world shader aborting\n");
        return -1;
    }

    vinoxCreateBuffer(&vinGLState.buffer);

    glUseProgram(screenProgram.shaderID);
    vinoxCreateFramebuffer(&vinGLState.frameBuffer, width, height);
    /* Set the screen texture to our framebuffer texture in shader */
    unsigned int loc = glGetUniformLocation(screenProgram.shaderID, "screenTexture");
    glUniform1i(loc, 1);
    printf("Framebuffer Texture ID: %i\n", vinGLState.frameBuffer.textureColorbuffer);
    
    glUseProgram(program.shaderID);
    
    unsigned int loc2 = glGetUniformLocation(program.shaderID, "uTextures");
    int textures[8] = { 0, 2, 3, 4, 5, 6, 7 }; // The first slot is set to 0 just to fill it
                                   // It is reserved for the framebuffer texture
    glUniform1iv(loc2, 3, textures);

    return 0;
}

void vinoxBeginDrawing(Camera camera, int width, int height) {
    buffer = vinGLState.buffer.vertices;
    indexCount = 0;
    vertexCount = 0;
    glBindFramebuffer(GL_FRAMEBUFFER, vinGLState.frameBuffer.fbo);
    glEnable(GL_DEPTH_TEST);
    vinoxResizeFramebuffer(&vinGLState.frameBuffer, width, height);
    glViewport(0, 0, width, height);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(program.shaderID);
    
    mat4 viewprojection = GLM_MAT4_IDENTITY_INIT;
    vinoxCameraMatrix(viewprojection, &camera, width, height);
    glUniformMatrix4fv(glGetUniformLocation(program.shaderID, "projection"), 1, false, viewprojection[0]);
}

int vinoxCreateQuad(float x, float y, float width, float height, float textureID, vec4 color) {
        
        //TODO: figure out how to draw when we start the new drawCall
        //IDEA: set buffer to vertices after we push the vertices array
        //static Vertex vertices[4000];
        //Vertex *buffer = vertices;
        drawCalls = vertexCount/(MAXVERTEXCOUNT);
        currentDrawCall = drawCalls;
        int id = 1;
        if (textureID == 0)
            id = 1;
        else
            id = textureID;

        if ((int)((vertexCount += 100)/(MAXVERTEXCOUNT)) > currentDrawCall) {
            drawBatch();
            memset(&vinGLState.buffer.vertices[0], 0, sizeof(vinGLState.buffer.vertices));
            buffer = vinGLState.buffer.vertices;
        }

        buffer = createQuad(buffer, x, y, width, height, id, color);
        indexCount += 6;

        lastDrawCall = currentDrawCall;
    return 0;
}

void vinoxEndDrawing() {
    
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
    
    drawBatch();
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(screenProgram.shaderID);
    glBindVertexArrayOES(vinGLState.frameBuffer.vao);
    glBindTexture(GL_TEXTURE_2D, vinGLState.frameBuffer.textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, vinGLState.frameBuffer.textureColorbuffer);
    glDrawArrays(GL_TRIANGLES, 0, 6);


    currentDrawCall = 0;
    lastDrawCall = 0;
    vertexCount = 0;
}   

int vinoxEnd() {

    return 0;
}

Vertex* createQuad(Vertex* target, float x, float y, float width, float height,
        float textureID, vec4 color) {
    
    glActiveTexture(GL_TEXTURE0 + textureID);   
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    glm_vec3_copy((vec3) {x, y, 0.0f }, target->position);
    glm_vec4_copy(color, target->color);
    glm_vec2_copy((vec2) { 0.0f, 0.0f }, target->texCoords);
    target->texIndex = textureID - 1;
    target++;

    glm_vec3_copy((vec3) {x + width, y, 0.0f }, target->position);
    glm_vec4_copy(color, target->color);
    glm_vec2_copy((vec2) { 1.0f, 0.0f }, target->texCoords);
    target->texIndex = textureID - 1;
    target++;

    glm_vec3_copy((vec3) {x + width, y + height, 0.0f }, target->position);
    glm_vec4_copy(color, target->color);
    glm_vec2_copy((vec2) { 1.0f, 1.0f }, target->texCoords);
    target->texIndex = textureID - 1;
    target++;

    glm_vec3_copy((vec3) {x, y + height, 0.0f }, target->position);
    glm_vec4_copy(color, target->color);
    glm_vec2_copy((vec2) { 0.0f, 1.0f }, target->texCoords);
    target->texIndex = textureID - 1;
    target++;

    vertexCount += 4;
    return target;
}
