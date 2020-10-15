#define EGL_NO_X11
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#define RAYMATH_IMPLEMENTATION
#include "raymath.h"
#include "vingl.h"
#include "camera.h"
#include "buffer.h"
#include "texture.h"
#include "shader.h"

/* Functions for the file */
static Vertex* createQuad(Vertex* target, Quad quad, Quad textureMask,
        float textureID, Vector4 color, float rotation);
static int drawBatchQuads();

/* Global Area */
typedef struct {
    Matrix matrix;
    Buffer quadBuffer;
    Buffer pointBuffer;
    ShaderProgram program;
    int width, height;
} vinState;

/* These are the vertex array extensions we need */
static PFNGLGENVERTEXARRAYSOESPROC glGenVertexArraysOES;
static PFNGLBINDVERTEXARRAYOESPROC glBindVertexArrayOES;
static PFNGLDELETEVERTEXARRAYSOESPROC glDeleteVertexArraysOES;

/* Global variables */
static vinState vinGLState = {0};
static Matrix defaultMatrix;
static Vertex* quadBuffer = vinGLState.quadBuffer.vertices;
static uint32_t curTextures[8] = { 0 };
/*static Vertex* pointBuffer = vinGLState.pointBuffer.vertices;*/

/* Counters */
static uint32_t indexCount = 0;
static uint32_t quadCount = 0;
static uint32_t drawCalls = 0;
static uint32_t textureCount = 1;

/* Clears color so we can do this for render textures as well */
int vinoxClear(Vector4 color) {
    glClearColor(color.x, color.y, color.z, color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    return 0;
}

static Matrix lastMatrix;
/* Begin a framebuffer and bind the current framebuffer to that one */
int vinoxBeginTexture(FrameBuffer *frameBuffer) {
    for (uint32_t i = 0; i < textureCount; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, curTextures[i]);
    }

    glBindVertexArrayOES(vinGLState.quadBuffer.vao);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer->fbo);
    vinoxResizeFramebuffer(frameBuffer);
    
    lastMatrix = vinGLState.matrix;
    vinGLState.matrix = MatrixOrtho(0, frameBuffer->texture.width, frameBuffer->texture.height, 0, -1.0f, 1.0f);
    glUniformMatrix4fv(glGetUniformLocation(vinGLState.program.shaderID, "projection"), 1, false, &MatrixToFloat(vinGLState.matrix)[0]);
    glViewport(0, 0, frameBuffer->texture.width, frameBuffer->texture.height);
    return 0;
}

/* Render all quads to the framebuffer and bind back to the default framebuffer
 * */
int vinoxEndTexture(FrameBuffer *frameBuffer) {
    drawBatchQuads();
    glViewport(0, 0, vinGLState.width, vinGLState.height);
    vinGLState.matrix = lastMatrix;
    glUniformMatrix4fv(glGetUniformLocation(vinGLState.program.shaderID, "projection"), 1, false, &MatrixToFloat(vinGLState.matrix)[0]);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return 0;
}

/* Begins using a user camera by changing the states camera */
int vinoxBeginCamera(Camera *camera) {
    Matrix viewprojection = vinoxCameraMatrix(camera, vinGLState.matrix);
    vinGLState.matrix = viewprojection;
    glUniformMatrix4fv(glGetUniformLocation(vinGLState.program.shaderID, "projection"), 1, false, &MatrixToFloat(vinGLState.matrix)[0]);
    
    return 0;
}

/* When we are done set the camera back to the default camera */
int vinoxEndCamera() {
    drawBatchQuads();
    vinGLState.matrix = defaultMatrix;
    glUniformMatrix4fv(glGetUniformLocation(vinGLState.program.shaderID, "projection"), 1, false, &MatrixToFloat(vinGLState.matrix)[0]);
    return 0;
}

/* This function draws the current batch of vertices and elements of quads
 * however in the future I may split this up into different functions depending
 * on the geometry type etc */
static int drawBatchQuads() {
    
    /* Bind our new vertex buffer to the vbo and send it to the gpu, then
     * attach Vertex array and draw */
    for (uint32_t i = 0; i < textureCount; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, curTextures[i]);
    }

    glBindBuffer(GL_ARRAY_BUFFER, vinGLState.quadBuffer.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * MAXVERTEXCOUNT, &vinGLState.quadBuffer.vertices[0]);
        
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    memset(&vinGLState.quadBuffer.vertices, 0, sizeof(vinGLState.quadBuffer.vertices));
    textureCount = 1;

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
    vinGLState.program.type = 0;
    if (vinoxCompileShader(&vinGLState.program) == -1) {
        printf("Failed to compile world shader aborting\n");
        return -1;
    }
    
    /* Create our buffers */
    vinoxCreateQuadBuffer(&vinGLState.quadBuffer);
    
    /* Don't bind anything to texture slot 0 */
    glActiveTexture(GL_TEXTURE0);   
    glBindTexture(GL_TEXTURE_2D, 0);
    
    /* Set up a default camera */
    defaultMatrix = MatrixOrtho(0.0f, width, height, 0.0f, -1.0f, 1.0f);
    vinGLState.matrix = defaultMatrix;
    
    curTextures[0] = 0;
    
    /* Assign texture ids to the array inside of the shader program */
    glUseProgram(vinGLState.program.shaderID);

    unsigned int loc2 = glGetUniformLocation(vinGLState.program.shaderID, "uTextures");
    int textures[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
    glUniform1iv(loc2, 8, textures);

    return 0;
}

/* This is called to actually intialize things that need to be done every frame
 * such as clearing the color */
void vinoxBeginDrawing(int width, int height) {
    
    /* Reassign the buffer pointer to the vertices for editing */
    glBindVertexArrayOES(vinGLState.quadBuffer.vao);
    quadBuffer = vinGLState.quadBuffer.vertices;
    vinGLState.width = width;
    vinGLState.height = height;

    textureCount = 1;

    /* Now just stuff to get ready for rendering and our matrix to be able to
     * provide a camera */
    glViewport(0, 0, width, height);
    glUseProgram(vinGLState.program.shaderID);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    /* Set the default matrix if begin camera is called after this it will
     * overide it */
    defaultMatrix = MatrixOrtho(0.0f, width, height, 0.0f, -1.0f, 1.0f);
    vinGLState.matrix = defaultMatrix;
    glUniformMatrix4fv(glGetUniformLocation(vinGLState.program.shaderID, "projection"), 1, false, &MatrixToFloat(vinGLState.matrix)[0]);

}

/* Finishes rendering to the framebuffer and makes sure all batches are drawn */
void vinoxEndDrawing() {
    
    /* Make sure we finish drawing the current batch */
    drawBatchQuads();
    
    /* Reset counters to 0 */
    indexCount = 0;
    quadCount = 0;
}

int vinoxEnd() {
    
    return 0;
}

/* Shapes */

/* This is called by the user just is basically a wrapper around the createQuad
 * in vingl however has some checking for when to split up a batch */
int vinoxCreateQuad(Quad quad, Quad textureMask, float textureID, Vector4 color, float rotation) {
        
        /* Detect how many drawcalls are needed for vertex count */
        drawCalls = quadCount/(MAXQUADCOUNT);
        
        /* This checks to see how many textures are needed and if it is more
         * then the max available start a new drawcall and reset */
        float textureIndex = 0.0f;
        if (textureID != 0) {
            for (uint32_t i = 1; i < textureCount; i++) {
                if (curTextures[i] == textureID) {
                    textureIndex = (float)i;
                    break;
                }
            }

            if (textureIndex == 0.0f) {
                if (textureCount >= 8)
                    drawBatchQuads();
                
                textureIndex = (float)textureCount;
                curTextures[textureCount] = textureID;
                textureCount++;
            }
        }

        /* Detect if adding any more vertices will cause a new batch to be made
         * if so drawThe current batch and reset buffer */
        if ((int)((quadCount + 1)/(MAXQUADCOUNT)) > (int)drawCalls) {
            drawBatchQuads();
            quadBuffer = vinGLState.quadBuffer.vertices;
        }
        
        /* Assiging vertex data to our vertices array */
        quadBuffer = createQuad(quadBuffer, quad, textureMask, textureIndex, color, rotation);
        indexCount += 6;

    return 0;
}

/* Function to assign data to each vertex for every quad in the vertices array
 * */
Vertex* createQuad(Vertex* target, Quad quad, Quad textureMask, float textureID,
        Vector4 color, float rotation) {
    
    /* We use this to avoid having 4 seperate sections */
    Vector2 quadVertices[4] = {
    (Vector2) { -0.5f, -0.5f },
    (Vector2) { 0.5f, -0.5f },
    (Vector2) { 0.5f, 0.5f },
    (Vector2) { -0.5f, 0.5f }};

    
    /* Temporarily have it use the full texture coords however we can take in a
     * rectangle and change this array acordingly */
    Vector2 textureCoords[4] = {
    (Vector2) { 0.0f, 0.0f },
    (Vector2) { 1.0f, 0.0f },
    (Vector2) { 1.0f, 1.0f },
    (Vector2) { 0.0f, 1.0f }};

    /* We use matrices to transform the vertices now to make it easier to do so
     * especially rotating */

    /* Currently this is the most performance bottlknecked area due to all the
     * matrix math for now I think it is fine but something to note to look in
     * for the future */
    /* One idea could be two do the position and scale our selves like
     * previously to avoid two multiply calls then only do rotation and use our
     * Vector2Transform for the speed up*/
    Matrix transform = MatrixIdentity();
    Matrix translate = MatrixTranslate(quad.position.x, quad.position.y, 0.0f);
    transform = MatrixMultiply(translate, transform);
    
    Vector3 axis = (Vector3) {0.0f, 0.0f, 1.0f};
    Matrix rotate = MatrixRotate(Vector3Normalize(axis), rotation*DEG2RAD);
    transform = MatrixMultiply(rotate, transform);

    Matrix scale = MatrixScale(quad.size.x, quad.size.y, 1.0f);
    transform = MatrixMultiply(scale, transform);

    /* Sets all of the needed data for current vertex then moves onto the next
     * 3*/
    for (int i = 0; i < 4; i++) {
        Vector2 vertex = Vector2Transform((Vector2) { quadVertices[i].x, quadVertices[i].y }, transform);
        target->position = (Vector3) { vertex.x, vertex.y, 0.0f };
        target->color = color;
        target->texCoords = (Vector2) { textureCoords[i].x, textureCoords[i].y };
        target->texIndex = textureID;
        target++;
    }

    quadCount++;
    return target;
}
