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
static Vertex* createQuad(Vertex* target, Quad quad, Quad textureMask,
        float textureID, Vector4 color, float rotation);
static int drawBatchQuads();

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

/* Global variables */
static vinState vinGLState = {0};
static Matrix defaultMatrix;
static Vertex* buffer = vinGLState.buffer.vertices;
static unsigned int loc;

/* Counters */
static uint32_t indexCount = 0;
static uint32_t quadCount = 0;
static uint32_t drawCalls = 0;

/* Clears color so we can do this for render textures as well */
int vinoxClear(Vector4 color) {
    glClearColor(color.x, color.y, color.z, color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    return 0;
}

static Matrix lastMatrix;
/* Begin a framebuffer and bind the current framebuffer to that one */
int vinoxBeginTexture(FrameBuffer *frameBuffer) {
    for (int i = 1; i <= 8; i++) {
        glActiveTexture(GL_TEXTURE0 + i);   
        glBindTexture(GL_TEXTURE_2D, i);
    }

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
    glBindFramebuffer(GL_FRAMEBUFFER, vinGLState.frameBuffer.fbo);
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
    glActiveTexture(GL_TEXTURE0 + vinGLState.frameBuffer.texture.id);
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
    vinGLState.program.type = 0;
    if (vinoxCompileShader(&vinGLState.program) == -1) {
        printf("Failed to compile world shader aborting\n");
        return -1;
    }

    vinGLState.screenProgram.type = 1;
    if (vinoxCompileShader(&vinGLState.screenProgram) == -1) {
        printf("Failed to compile world shader aborting\n");
        return -1;
    }
    
    /* Create our buffers */
    vinoxCreateBuffer(&vinGLState.buffer);
    
    /* Don't bind anything to texture slot 0 */
    glActiveTexture(GL_TEXTURE0);   
    glBindTexture(GL_TEXTURE_2D, 0);
    
    glUseProgram(vinGLState.screenProgram.shaderID);
    vinGLState.frameBuffer.texture.width = width;
    vinGLState.frameBuffer.texture.height = height;
    vinGLState.frameBuffer.scaleType = GL_LINEAR;
    vinoxCreateFramebuffer(&vinGLState.frameBuffer);

    /* Set up a default camera */
    defaultMatrix = MatrixOrtho(0.0f, width, height, 0.0f, -1.0f, 1.0f);
    vinGLState.matrix = defaultMatrix;
    
    /* Set the screen texture to our framebuffer texture in shader */
    loc = glGetUniformLocation(vinGLState.screenProgram.shaderID, "screenTexture");
    glUniform1i(loc, vinGLState.frameBuffer.texture.id);
    printf("Framebuffer Texture ID: %i\n", vinGLState.frameBuffer.texture.id);
    
    /* Assign texture ids to the array inside of the shader program */
    glUseProgram(vinGLState.program.shaderID);
    glBindVertexArrayOES(vinGLState.buffer.vao);

    unsigned int loc2 = glGetUniformLocation(vinGLState.program.shaderID, "uTextures");
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
    
    /*Bind all the textures*/
    for (int i = 1; i <= 8; i++) {
        glActiveTexture(GL_TEXTURE0 + i);   
        glBindTexture(GL_TEXTURE_2D, i);
    }

    /* Switch over to drawing to our framebuffer */
    glBindFramebuffer(GL_FRAMEBUFFER, vinGLState.frameBuffer.fbo);
    vinGLState.frameBuffer.texture.width = width;
    vinGLState.frameBuffer.texture.height = height;
    vinoxResizeFramebuffer(&vinGLState.frameBuffer);
    
    /* Now just stuff to get ready for rendering and our matrix to be able to
     * provide a camera */
    glViewport(0, 0, width, height);
    glUseProgram(vinGLState.program.shaderID);
    
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
    
    /* Switch back to drawing to the context */
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    /* Draw our framebuffer texture to the screen */
    glUseProgram(vinGLState.screenProgram.shaderID);
    glBindVertexArrayOES(vinGLState.frameBuffer.vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glBindVertexArrayOES(vinGLState.buffer.vao);

    /* Reset counters to 0 */
    indexCount = 0;
    quadCount = 0;
}   

/* TODO: uninit and destroy everything */
int vinoxEnd() {

    return 0;
}

/* Shapes */

/* This is called by the user just is basically a wrapper around the createQuad
 * in vingl however has some checking for when to split up a batch */
int vinoxCreateQuad(Quad quad, Quad textureMask, float textureID, Vector4 color, float rotation) {
        
        /* Detect how many drawcalls are needed for vertex count */
        drawCalls = quadCount/(MAXQUADCOUNT);
        
        /* Detect if adding any more vertices will cause a new batch to be made
         * if so drawThe current batch and reset buffer */
        if ((int)((quadCount + 1)/(MAXQUADCOUNT)) > (int)drawCalls) {
            drawBatchQuads();
            buffer = vinGLState.buffer.vertices;
        }
        /* Assiging vertex data to our vertices array */
        buffer = createQuad(buffer, quad, textureMask, textureID, color, rotation);
        indexCount += 6;

    return 0;
}


/* Raylib does not provide a vector2transform so we make our own from my testing
 * this improved performance around 4% to 5% */
Vector2 Vector2Transform(Vector2 v, Matrix mat) {

        Vector2 result = { 0 };
        float x = v.x;
        float y = v.y;

        result.x = mat.m0*x + mat.m4*y + mat.m12;
        result.y = mat.m1*x + mat.m5*y + mat.m13;

        return result;
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
