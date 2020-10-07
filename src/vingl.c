#define EGL_NO_X11
#include <EGL/egl.h>
#define GL_GLEXT_PROTOTYPES 1
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <stdio.h>
#include "vingl.h"
#include "shader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

/* Functions for the file */
static Vertex* createQuad(Vertex* target, float x, float y, float width, float height,
        float textureID, vec4 color);

/* Global variables in file */
static ShaderProgram program;
static GLuint vao;
static GLuint vbo;
static uint32_t indexCount = 0;
static uint32_t vertexCount = 0;
static uint32_t drawCalls = 0;
#define MAXQUADCOUNT 20000
#define MAXVERTEXCOUNT MAXQUADCOUNT * 4
#define INDICESCOUNT MAXQUADCOUNT * 6
static Vertex vertices[MAXVERTEXCOUNT];
static Vertex* buffer = vertices;
static int currentDrawCall = 0;
static int lastDrawCall = 0;
/*const char *getGLError(GLenum err) {
    switch (err) {
        case GL_NO_ERROR:   return "No error";
        case GL_INVALID_ENUM:   return "Invalid name";
        case GL_INVALID_VALUE:  return "Invalid value";
        case GL_STACK_OVERFLOW_KHR: return "Stack overflow";
        case GL_STACK_UNDERFLOW_KHR: return "Stack underflow";
        case GL_OUT_OF_MEMORY:  return "Out of mem";
        default:    return "Unkown error";
    }
}*/

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
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * MAXVERTEXCOUNT, &vertices[0]);
        
    glBindVertexArrayOES(vao);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);

    return 0;
}

static int calculateCameraMatrix(mat4 viewprojection, Camera *camera, int width, int height) {

    /* Camera transformations */
    mat4 projection = GLM_MAT4_IDENTITY_INIT;
    mat4 view = GLM_MAT4_IDENTITY_INIT;
    glm_ortho(0.0f, width, height, 0.0f, -1.0f, 1.0f, projection);
        
    /* Camera origin */
    mat4 position = GLM_MAT4_IDENTITY_INIT;
    glm_translate(position, (vec3) { camera->origin.x, camera->origin.y, 0.0f });
        
    /* Camera rotation */
    glm_rotate(position, glm_rad(camera->rotation), (vec3) { 0.0f, 0.0f, 1.0f });
        
    /* Camera zoom */
    glm_scale(position, (vec3) { camera->scale, camera->scale, 1.0f });
        
    /* Camera position */
    vec3 camPosition = { -camera->position.x, -camera->position.y, 0.0f };
    glm_translate(position, camPosition);
        
        
    //glm_mat4_mul(position, rotation, view);
    glm_mat4_copy(position, view);
    glm_mat4_mul(projection, view, viewprojection);
    
    return 0;
}

static int createBuffer() {

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
    
    glGenVertexArraysOES(1, &vao);

    glGenBuffers(1, &vbo);

    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindVertexArrayOES(vao);

    

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
    
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * MAXVERTEXCOUNT, 
            NULL, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    return 0;
}

int vinoxInit() {
    
    if (vinoxCompileShader(&program) == -1) {
        printf("Failed to compile ShaderProgram aborting\n");
        return -1;
    }
    glUseProgram(program.shaderID);

    unsigned int containerTex = vinoxCreateTexture("container.jpg");
    unsigned int containerTex2 = vinoxCreateTexture("container.jpg");
    unsigned int loc = glGetUniformLocation(program.shaderID, "uTextures");
    int samplers[2] = { 0, 1 };
    glUniform1iv(loc, 2, samplers);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, containerTex2);

    createBuffer();

    /* Set up our vertex pointers so we can send our positions and such
     * dynamically */
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
            (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
            (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(1);
    
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
            (void*)offsetof(Vertex, texCoords));
    glEnableVertexAttribArray(2);
    
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex),
            (void*)offsetof(Vertex, texIndex));
    glEnableVertexAttribArray(3);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return 0;
}

void vinoxBeginDrawing(Camera camera, int width, int height) {
    buffer = vertices;
    indexCount = 0;
    vertexCount = 0;
    glViewport(0, 0, width, height);

    mat4 viewprojection = GLM_MAT4_IDENTITY_INIT;
    calculateCameraMatrix(viewprojection, &camera, width, height);
    glUniformMatrix4fv(glGetUniformLocation(program.shaderID, "projection"), 1, false, viewprojection[0]);
    
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

int vinoxCreateQuad(float x, float y, float width, float height, float textureID, vec4 color) {
        
        //TODO: figure out how to draw when we start the new drawCall
        //IDEA: set buffer to vertices after we push the vertices array
        //static Vertex vertices[4000];
        //Vertex *buffer = vertices;
        drawCalls = vertexCount/(MAXVERTEXCOUNT);
        currentDrawCall = drawCalls;
         
        if ((int)((vertexCount += 100)/(MAXVERTEXCOUNT)) > currentDrawCall) {
            drawBatch();
            memset(&vertices[0], 0, sizeof(vertices));
            buffer = vertices;
        }

        buffer = createQuad(buffer, x, y, width, height, textureID, color);
        indexCount += 6;

        lastDrawCall = currentDrawCall;
    return 0;
}

void vinoxEndDrawing() {
    
    drawBatch();

    currentDrawCall = 0;
    lastDrawCall = 0;
    vertexCount = 0;
}   

int vinoxEnd() {

    return 0;
}

unsigned int vinoxCreateTexture(const char* path) {

    int w, h, c;
    /* Load the texture we should error check here */
    unsigned char *data = stbi_load(path, &w, &h, &c, 0);
    
    /* Generate the GLtexture and free the texture after we are done */
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
    
    return textureID;
}

Vertex* createQuad(Vertex* target, float x, float y, float width, float height,
        float textureID, vec4 color) {
    
    glm_vec3_copy((vec3) {x, y, 0.0f }, target->position);
    glm_vec4_copy(color, target->color);
    glm_vec2_copy((vec2) { 0.0f, 0.0f }, target->texCoords);
    target->texIndex = textureID;
    target++;

    glm_vec3_copy((vec3) {x + width, y, 0.0f }, target->position);
    glm_vec4_copy(color, target->color);
    glm_vec2_copy((vec2) { 1.0f, 0.0f }, target->texCoords);
    target->texIndex = textureID;
    target++;

    glm_vec3_copy((vec3) {x + width, y + height, 0.0f }, target->position);
    glm_vec4_copy(color, target->color);
    glm_vec2_copy((vec2) { 1.0f, 1.0f }, target->texCoords);
    target->texIndex = textureID;
    target++;

    glm_vec3_copy((vec3) {x, y + height, 0.0f }, target->position);
    glm_vec4_copy(color, target->color);
    glm_vec2_copy((vec2) { 0.0f, 1.0f }, target->texCoords);
    target->texIndex = textureID;
    target++;

    vertexCount += 4;
    return target;
}
