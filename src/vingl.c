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
        float textureID, Vector4 color, float rotation);
static int drawBatch();

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

/* Some variables our two shaderPrograms and our state */
static ShaderProgram program;
static ShaderProgram screenProgram;
static vinState vinGLState = {0};
static Matrix defaultMatrix;
static Vertex* buffer = vinGLState.buffer.vertices;
static unsigned int loc;

/* Counters */
static uint32_t indexCount = 0;
static uint32_t vertexCount = 0;
static uint32_t drawCalls = 0;
static int currentDrawCall = 0;
static int lastDrawCall = 0;

/* Clears color so we can do this for render textures as well */
int vinoxClear(Vector4 color) {
    glClearColor(color.x, color.y, color.z, color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    return 0;
}

static Matrix lastMatrix;
/* Begin a framebuffer and bind the current framebuffer to that one */
int vinoxBeginTexture(FrameBuffer *frameBuffer) {
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer->fbo);
    vinoxResizeFramebuffer(frameBuffer);
    
    lastMatrix = vinGLState.matrix;
    vinGLState.matrix = MatrixOrtho(0, frameBuffer->texture.width, frameBuffer->texture.height, 0, -1.0f, 1.0f);
    glUniformMatrix4fv(glGetUniformLocation(program.shaderID, "projection"), 1, false, &MatrixToFloat(vinGLState.matrix)[0]);
    glViewport(0, 0, frameBuffer->texture.width, frameBuffer->texture.height);
    return 0;
}

/* Render all quads to the framebuffer and bind back to the default framebuffer
 * */
int vinoxEndTexture(FrameBuffer *frameBuffer) {
    drawBatch();
    glViewport(0, 0, vinGLState.width, vinGLState.height);
    vinGLState.matrix = lastMatrix;
    glUniformMatrix4fv(glGetUniformLocation(program.shaderID, "projection"), 1, false, &MatrixToFloat(vinGLState.matrix)[0]);
    glBindFramebuffer(GL_FRAMEBUFFER, vinGLState.frameBuffer.fbo);
    return 0;
}

/* Begins using a user camera by changing the states camera */
int vinoxBeginCamera(Camera *camera) {
    Matrix viewprojection = vinoxCameraMatrix(camera, vinGLState.matrix);
    vinGLState.matrix = viewprojection;
    glUniformMatrix4fv(glGetUniformLocation(program.shaderID, "projection"), 1, false, &MatrixToFloat(vinGLState.matrix)[0]);
    
    return 0;
}

/* When we are done set the camera back to the default camera */
int vinoxEndCamera() {
    //vinGLState.matrix = defaultMatrix;
    //glUniformMatrix4fv(glGetUniformLocation(program.shaderID, "projection"), 1, false, &MatrixToFloat(vinGLState.matrix)[0]);
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
    memset(&vinGLState.buffer.vertices[0], 0, sizeof(vinGLState.buffer.vertices));

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
    
    /* Don't bind anything to texture slot 0 */
    glActiveTexture(GL_TEXTURE0);   
    glBindTexture(GL_TEXTURE_2D, 0);

    /* Create our buffers */
    vinoxCreateBuffer(&vinGLState.buffer);
    
    glUseProgram(screenProgram.shaderID);
    vinGLState.frameBuffer.texture.width = width;
    vinGLState.frameBuffer.texture.height = height;
    vinoxCreateFramebuffer(&vinGLState.frameBuffer);

    /* Set up a default camera */
    defaultMatrix = MatrixOrtho(0.0f, width, height, 0.0f, -1.0f, 1.0f);
    vinGLState.matrix = defaultMatrix;
    
    /* Set the screen texture to our framebuffer texture in shader */
    loc = glGetUniformLocation(screenProgram.shaderID, "screenTexture");
    glUniform1i(loc, vinGLState.frameBuffer.texture.id);
    printf("Framebuffer Texture ID: %i\n", vinGLState.frameBuffer.texture.id);
    
    /* Assign texture ids to the array inside of the shader program */
    glUseProgram(program.shaderID);
    
    unsigned int loc2 = glGetUniformLocation(program.shaderID, "uTextures");
    int textures[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
    glUniform1iv(loc2, 8, textures);

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
    glUseProgram(program.shaderID);
    
    /* Set the default matrix if begin camera is called after this it will
     * overide it */
    defaultMatrix = MatrixOrtho(0.0f, width, height, 0.0f, -1.0f, 1.0f);
    vinGLState.matrix = defaultMatrix;
    glUniformMatrix4fv(glGetUniformLocation(program.shaderID, "projection"), 1, false, &MatrixToFloat(vinGLState.matrix)[0]);

}

/* Finishes rendering to the framebuffer and makes sure all batches are drawn */
void vinoxEndDrawing() {
    
    /* Make sure we finish drawing the current batch */
    drawBatch();
    
    /* Switch back to drawing to the context */
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    /* Draw our framebuffer texture to the screen */
    glUseProgram(screenProgram.shaderID);
    glBindVertexArrayOES(vinGLState.frameBuffer.vao);
    glActiveTexture(vinGLState.frameBuffer.texture.id);
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
int vinoxCreateQuad(float x, float y, float width, float height, float textureID, Vector4 color, float rotation) {
        
        /* Detect how many drawcalls are needed for vertex count */
        drawCalls = vertexCount/(MAXVERTEXCOUNT);
        currentDrawCall = drawCalls;
        
        /* Detect if adding any more vertices will cause a new batch to be made
         * if so drawThe current batch and reset buffer */
        if ((int)((vertexCount += 100)/(MAXVERTEXCOUNT)) > currentDrawCall) {
            drawBatch();
            buffer = vinGLState.buffer.vertices;
        }
        
        /* Assiging vertex data to our vertices array */
        buffer = createQuad(buffer, x, y, width, height, textureID, color, rotation);
        indexCount += 6;

        lastDrawCall = currentDrawCall;
    return 0;
}


/* Function to assign data to each vertex for every quad in the vertices array
 * */
Vertex* createQuad(Vertex* target, float x, float y, float width, float height,
        float textureID, Vector4 color, float rotation) {
    
    /* Set the active texture and bind it to whichever id is passed */
    /* This is kind of jank but I haven't fixed it yet so here it will stay for
     * now */
    if (!(textureID == 0)) {
    glActiveTexture(GL_TEXTURE0 + textureID);   
    glBindTexture(GL_TEXTURE_2D, textureID);
    }else{
    glActiveTexture(GL_TEXTURE0 + vinGLState.frameBuffer.texture.id);
    glBindTexture(GL_TEXTURE_2D, vinGLState.frameBuffer.texture.id);
    }
    
    /* We use matrices to transform the vertices now to make it easier to do so
     * especially rotating */
    Matrix transform = MatrixIdentity();
    Matrix translate = MatrixTranslate(x, y, 0.0f);
    transform = MatrixMultiply(translate, transform);
    
    Vector3 axis = (Vector3) {0.0f, 0.0f, 1.0f};
    Matrix rotate = MatrixRotate(Vector3Normalize(axis), rotation*DEG2RAD);
    transform = MatrixMultiply(rotate, transform);

    Matrix scale = MatrixScale(width, height, 1.0f);
    transform = MatrixMultiply(scale, transform);

    /* Sets all of the needed data for current vertex then moves onto the next
     * 3*/
    Vector3 vertex1 = Vector3Transform((Vector3) { -0.5f, -0.5f, 0.0f }, transform);
    target->position = (Vector3) { vertex1.x, vertex1.y, 0.0f };
    target->color = color;
    target->texCoords = (Vector2) { 0.0f, 0.0f };
    target->texIndex = textureID;
    target++;

    Vector3 vertex2 = Vector3Transform((Vector3) { 0.5f, -0.5f, 0.0f }, transform);
    target->position = (Vector3) { vertex2.x, vertex2.y, 0.0f };
    target->color = color;
    target->texCoords = (Vector2) { 1.0f, 0.0f };
    target->texIndex = textureID;
    target++;

    Vector3 vertex3 = Vector3Transform((Vector3) { 0.5f, 0.5f, 0.0f }, transform);
    target->position = (Vector3) { vertex3.x, vertex3.y, 0.0f };
    target->color = color;
    target->texCoords = (Vector2) { 1.0f, 1.0f };
    target->texIndex = textureID;
    target++;

    Vector3 vertex4 = Vector3Transform((Vector3) { -0.5f, 0.5f, 0.0f }, transform);
    target->position = (Vector3) { vertex4.x, vertex4.y, 0.0f };
    target->color = color;
    target->texCoords = (Vector2) { 0.0f, 1.0f };
    target->texIndex = textureID;
    target++;


    vertexCount += 4;
    return target;
}
