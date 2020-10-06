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

#define WHITE (Vec4) { 1.0f, 1.0f, 1.0f, 1.0f }


/* Global variables in file */
static ShaderProgram program;
static GLuint vao;
static GLuint vbo;
static uint32_t indexCount = 0;

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

    const size_t maxQuadCount = 1000;
    const size_t maxVertexCount = maxQuadCount * 4;
    const size_t indicesCount = maxQuadCount * 6;
    

    /* This preallocates our max indices for every draw call */
    uint32_t indices[indicesCount];
    uint32_t offset = 0;
    
    for (size_t i = 0; i < indicesCount; i += 6) {
        indices[i + 0] = 0 + offset;
        indices[i + 1] = 1 + offset;
        indices[i + 2] = 2 + offset;
        
        indices[i + 3] = 2 + offset;
        indices[i + 4] = 3 + offset;
        indices[i + 5] = 0 + offset;
        
        offset += 4;
    }
    
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * maxVertexCount, 
            NULL, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, ebo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
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


    return 0;
}

void vinoxBeginDrawing(Camera camera, int width, int height) {
    Vertex vertices[1000];
    Vertex* buffer = vertices;

    glUseProgram(program.shaderID);
    /* to test it for now draw one texture */
    for (int y = -6; y < 5; y++) {
        for (int x = -6; x < 5; x++) {
            buffer = vinoxCreateQuad(buffer, x * 100, y * 100, 100.0f, 100.0f, (x + y) % 2, WHITE);
            indexCount += 6;
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices[0]);

    mat4 viewprojection = GLM_MAT4_IDENTITY_INIT;
        
    /* Camera transformations */
    mat4 projection = GLM_MAT4_IDENTITY_INIT;
    mat4 view = GLM_MAT4_IDENTITY_INIT;
    glm_ortho(0.0f, width, height, 0.0f, -1.0f, 1.0f, projection);
        
    /* Camera origin */
    mat4 position = GLM_MAT4_IDENTITY_INIT;
    glm_translate(position, (vec3) { 0.0f, 0.0f, 0.0f });
        
    /* Camera rotation */
    glm_rotate(position, glm_rad(0.0f), (vec3) { 0.0f, 0.0f, 1.0f });
        
    /* Camera zoom */
    glm_scale(position, (vec3) { 1.0f, 1.0f, 1.0f });
        
    /* Camera position */
    vec3 camPosition = { width/2, height/2, 0.0f };
    glm_translate(position, camPosition);
        
        
    //glm_mat4_mul(position, rotation, view);
    glm_mat4_copy(position, view);
    glm_mat4_mul(projection, view, viewprojection);

    glUniformMatrix4fv(glGetUniformLocation(program.shaderID, "projection"), 1, false, viewprojection[0]);
    
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
}


void vinoxEndDrawing() {

    glUseProgram(program.shaderID);
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
    
    glBindVertexArrayOES(vao);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
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

Vertex* vinoxCreateQuad(Vertex* target, float x, float y, float width, float height,
        float textureID, Vec4 color) {

    target->position = (Vec3) { x, y, 0.0f };
    target->color = color;
    target->texCoords = (Vec2) { 0.0f, 0.0f };
    target->texIndex = textureID;
    target++;

    target->position = (Vec3) { x + width, y, 0.0f };
    target->color = color;
    target->texCoords = (Vec2) { 1.0f, 0.0f };
    target->texIndex = textureID;
    target++;

    target->position = (Vec3) { x + width, y + height, 0.0f };
    target->color = color;
    target->texCoords = (Vec2) { 1.0f, 1.0f };
    target->texIndex = textureID;
    target++;

    target->position = (Vec3) { x, y + height, 0.0f };
    target->color = color;
    target->texCoords = (Vec2) { 0.0f, 1.0f };
    target->texIndex = textureID;
    target++;
    
    return target;
}
