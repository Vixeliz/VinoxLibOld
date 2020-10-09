#define EGL_NO_X11
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#define RAYMATH_IMPLEMENTATION
#include "raymath.h"
#include "vingl.h"
#include "camera.h"
#include "buffer.h"
#include "texture.h"
#include "shader.h"

/* Functions for the file */
static Vertex* createQuad(Vertex* target, float x, float y, float width, float height,
        float textureID, Vector4 color);

/* Global Area */
typedef struct {
    Matrix matrix;
    Buffer buffer;
    FrameBuffer frameBuffer;
    ShaderProgram program;
    ShaderProgram screenProgram;
    int width, height;
} vinState;

/* These are the vertex array extensions we need */
static PFNGLGENVERTEXARRAYSOESPROC glGenVertexArraysOES;
static PFNGLBINDVERTEXARRAYOESPROC glBindVertexArrayOES;
static PFNGLDELETEVERTEXARRAYSOESPROC glDeleteVertexArraysOES;

/* Some variables our two shaderPrograms, a state and counters basically */
static ShaderProgram program;
static ShaderProgram screenProgram;
static Matrix defaultMatrix;
static vinState vinGLState = {0};
static uint32_t indexCount = 0;
static uint32_t vertexCount = 0;
static uint32_t drawCalls = 0;
static Vertex* buffer = vinGLState.buffer.vertices;
static int currentDrawCall = 0;
static int lastDrawCall = 0;
static unsigned int loc;

int vinoxClear(Vector4 color) {
    glClearColor(color.x, color.y, color.z, color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    return 0;
}

int vinoxBeginTexture(FrameBuffer *frameBuffer) {
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer->fbo);
    vinoxResizeFramebuffer(frameBuffer);
    
    return 0;
}


int vinoxEndTexture(FrameBuffer *frameBuffer) {
    
    glBindFramebuffer(GL_FRAMEBUFFER, vinGLState.frameBuffer.fbo);
    return 0;
}

int vinoxBeginCamera(Camera *camera) {
    Matrix viewprojection = vinoxCameraMatrix(camera, vinGLState.width, vinGLState.height);
    vinGLState.matrix = viewprojection;
    glUniformMatrix4fv(glGetUniformLocation(program.shaderID, "projection"), 1, false, &MatrixToFloat(vinGLState.matrix)[0]);
    
    return 0;
}

int vinoxEndCamera() {
    vinGLState.matrix = defaultMatrix;
    return 0;
}

/* This function draws the current batch of vertices and elements of quads
 * however in the future I may split this up into different functions depending
 * on the geometry type etc */
static int drawBatch() {
    
    /* Bind our new vertex buffer to the vbo and send it to the gpu, then
     * attach Vertex array and draw */
    glBindBuffer(GL_ARRAY_BUFFER, vinGLState.buffer.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * MAXVERTEXCOUNT, &vinGLState.buffer.vertices[0]);
        
    glBindVertexArrayOES(vinGLState.buffer.vao);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);

    return 0;
}

int vinoxInit(int width, int height) {
    
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
    
    /* Enable alpha for transparency */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    /* Compile our two shaderPrograms setting their types (we have one for
     * typical objects and a more basic shader for the framebuffer textures) */
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
    vinGLState.frameBuffer.texture.width = width;
    vinGLState.frameBuffer.texture.height = height;
    vinoxCreateFramebuffer(&vinGLState.frameBuffer);

    defaultMatrix = MatrixOrtho(0.0f, width, height, 0.0f, -1.0f, 1.0f);
    vinGLState.matrix = defaultMatrix;
    /* Set the screen texture to our framebuffer texture in shader */
    loc = glGetUniformLocation(screenProgram.shaderID, "screenTexture");
    glUniform1i(loc, vinGLState.frameBuffer.texture.id);
    printf("Framebuffer Texture ID: %i\n", vinGLState.frameBuffer.texture.id);
    
    glUseProgram(program.shaderID);
    
    unsigned int loc2 = glGetUniformLocation(program.shaderID, "uTextures");
    int textures[8] = { 0, 2, 3, 4, 5, 6, 7 }; /* The first slot is set to 0 as it is reserved for non textured quads */
    glUniform1iv(loc2, 3, textures);

    return 0;
}

/* This is called to actually intialize things that need to be done every frame
 * such as clearing the color */
void vinoxBeginDrawing(int width, int height) {
    /* Reassign the buffer pointer to the vertices for editing */
    buffer = vinGLState.buffer.vertices;
    vinGLState.width = width;
    vinGLState.height = height;
    
    /* Switch over to drawing to our framebuffer */
    glBindFramebuffer(GL_FRAMEBUFFER, vinGLState.frameBuffer.fbo);
    vinGLState.frameBuffer.texture.width = width;
    vinGLState.frameBuffer.texture.height = height;
    vinoxResizeFramebuffer(&vinGLState.frameBuffer);
    
    /* Now just stuff to get ready for rendering and our matrix to be able to
     * provide a camera */
    glViewport(0, 0, width, height);
    //vinoxClear((Vector4) { 0.1f, 0.1f, 0.1f, 1.0f });
    glUseProgram(program.shaderID);
    
    defaultMatrix = MatrixOrtho(0.0f, width, height, 0.0f, -1.0f, 1.0f);
    vinGLState.matrix = defaultMatrix;
    glUniformMatrix4fv(glGetUniformLocation(program.shaderID, "projection"), 1, false, &MatrixToFloat(vinGLState.matrix)[0]);
}

/* Finishes rendering to the framebuffer and makes sure all batches are drawn */
void vinoxEndDrawing() {
    
    drawBatch();
    
    /* Switch back to drawing to the context */
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    /* Draw our framebuffer texture to the screen */
    glUseProgram(screenProgram.shaderID);
    glBindVertexArrayOES(vinGLState.frameBuffer.vao);
    glBindTexture(GL_TEXTURE_2D, vinGLState.frameBuffer.texture.id);
    glBindTexture(GL_TEXTURE_2D, vinGLState.frameBuffer.texture.id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    

    /* Reset counters to 0 */
    indexCount = 0;
    vertexCount = 0;
    currentDrawCall = 0;
    lastDrawCall = 0;
}   

/* TODO: uninit and destroy everything */
int vinoxEnd() {

    return 0;
}

/* Shapes */

/* This is called by the user just is basically a wrapper around the createQuad
 * in vingl however has some checking for when to split up a batch */
int vinoxCreateQuad(float x, float y, float width, float height, float textureID, Vector4 color) {
        /* Detect how many drawcalls are needed for vertex count */
        drawCalls = vertexCount/(MAXVERTEXCOUNT);
        currentDrawCall = drawCalls;
        /* This is just so if textureID is set to 0 it still shows up as color
         * */
        int id = 1;
        if (textureID == 0)
            id = 1;
        else
            id = textureID;
        
        /* Detect if adding any more vertices will cause a new batch to be made
         * if so drawThe current batch and reset vertices and buffer */
        if ((int)((vertexCount += 100)/(MAXVERTEXCOUNT)) > currentDrawCall) {
            drawBatch();
            memset(&vinGLState.buffer.vertices[0], 0, sizeof(vinGLState.buffer.vertices));
            buffer = vinGLState.buffer.vertices;
        }
        
        /* Assiging vertex data to our vertices array */
        buffer = createQuad(buffer, x, y, width, height, id, color);
        indexCount += 6;

        lastDrawCall = currentDrawCall;
    return 0;
}


/* Function to assign data to each vertex for every quad in the vertices array
 * */
Vertex* createQuad(Vertex* target, float x, float y, float width, float height,
        float textureID, Vector4 color) {
    
    /* Set the active texture and bind it to whichever id is passed */
    glActiveTexture(GL_TEXTURE0 + textureID);   
    glBindTexture(GL_TEXTURE_2D, textureID);

    /* Sets all of the needed data for current vertex then moves onto the next
     * 3*/
    target->position = (Vector3) { x, y, 0.0f };
    target->color = color;
    target->texCoords = (Vector2) { 0.0f, 0.0f };
    target->texIndex = textureID - 1;
    target++;

    target->position = (Vector3) { x + width, y, 0.0f };
    target->color = color;
    target->texCoords = (Vector2) { 1.0f, 0.0f };
    target->texIndex = textureID - 1;
    target++;

    target->position = (Vector3) { x + width, y + height, 0.0f };
    target->color = color;
    target->texCoords = (Vector2) { 1.0f, 1.0f };
    target->texIndex = textureID - 1;
    target++;

    target->position = (Vector3) { x, y + height, 0.0f };
    target->color = color;
    target->texCoords = (Vector2) { 0.0f, 1.0f };
    target->texIndex = textureID - 1;
    target++;


    vertexCount += 4;
    return target;
}
