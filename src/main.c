#define EGL_NO_X11
#include <EGL/egl.h>
#define GL_GLEXT_PROTOTYPES 1
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#define GLFW_NO_INCLUDE
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <cglm/cglm.h>

#include "shader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define WHITE (Vec4) { 1.0f, 1.0f, 1.0f, 1.0f }

typedef struct {
    float x, y;
} Vec2;
typedef struct {
    float x, y, z;
}Vec3;
typedef struct {
    float x, y, z, w;
}Vec4;

typedef struct {
    Vec3 position;
    Vec4 color;
    Vec2 texCoords;
    float texIndex;
} vertex;

Vec2 playerPos = (Vec2) { 0.1f, 0.25f };
float deltaTime;
float lastFrame;

void processInput(GLFWwindow *window) {
	float currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;
	float playerSpeed = 100.0f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			playerPos.y -= playerSpeed;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			playerPos.y += playerSpeed;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			playerPos.x -= playerSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			playerPos.x += playerSpeed;
}

static GLuint createTexture(const char* path) {
    int w, h, c;
    unsigned char *data = stbi_load(path, &w, &h, &c, 0);
    
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

static vertex* CreateQuad(vertex* target, float x, float y, float width, float height, float textureID, Vec4 color) {
    
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

int main(void) {

    /* Get VAO extension */
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

    GLFWwindow* window;
    glfwInit();

    window = glfwCreateWindow(640, 480, "Vinox", NULL, NULL);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    const size_t MaxQuadCount = 1000;
    const size_t MaxVertexCount = MaxQuadCount * 4;
    const size_t MaxIndexCount = MaxQuadCount * 6;


    /* Compile our shader program */
    ShaderProgram program;
    if (vinoxCompileShader(&program) == -1) {
        printf("Failed to compile shaderProgram aborting\n");
        return -1;
    }
    glUseProgram(program.shaderID);
    
    unsigned int containerTex = createTexture("container.jpg");
    unsigned int containerTex2 = createTexture("container.jpg");
    unsigned int loc = glGetUniformLocation(program.shaderID, "uTextures");
    int samplers[2] = { 0, 1 };
    glUniform1iv(loc, 2, samplers);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, containerTex2);

    GLuint vao;
    glGenVertexArraysOES(1, &vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);

    /*GLfloat vertices[] = {
        // positions     
        0.5f,  0.5f, 0.0f, 0.18f, 0.6f, 0.96f, 1.0f, 0.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.0f, 0.18f, 0.8f, 0.7f, 1.0f, 1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, 0.0f, 0.8f, 0.1f, 0.96f, 1.0f, 1.0f, 1.0f, 0.0f,
        -0.5f,  0.5f, 0.0f, 0.18f, 0.5f, 0.6f, 1.0f, 0.0f, 1.0f, 0.0f,

        -0.5f, 1.0f, 0.0f, 1.0f, 0.94f, 0.24f, 1.0f, 0.0f, 0.0f, 1.0f, 
        -0.5f, 0.5f, 0.0f, 1.0f, 0.7f, 0.1f, 1.0f, 1.0f, 0.0f, 1.0f,
        -1.0f, 0.5f, 0.0f, 0.7f, 0.84f, 0.04f, 1.0f, 1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 0.0f, 1.0f, 0.94f, 0.24f, 1.0f, 0.0f, 1.0f, 1.0f
        
    };*/
    /*GLuint indices[] = {
        0, 1, 3,
        1, 2, 3,

        4, 5, 7,
        5, 6, 7
    };*/
    uint32_t indices[MaxIndexCount];
    uint32_t offset = 0;

    for (size_t i = 0; i < MaxIndexCount; i += 6) {
        indices[i + 0] = 0 + offset;
        indices[i + 1] = 1 + offset;
        indices[i + 2] = 2 + offset;
        
        indices[i + 3] = 2 + offset;
        indices[i + 4] = 3 + offset;
        indices[i + 5] = 0 + offset;
        
        offset += 4;
    }

    GLuint EBO;
    glGenBuffers(1, &EBO);
    glBindVertexArrayOES(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * MaxVertexCount, NULL, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, position));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, color));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, texCoords));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, texIndex));
    glEnableVertexAttribArray(3);

    while (!glfwWindowShouldClose(window)) {
        int width, height;

        glfwGetFramebufferSize(window, &width, &height);
        processInput(window);

        /*GLfloat vertices[] = {
        // positions     
        0.5f,  0.5f, 0.0f, 0.18f, 0.6f, 0.96f, 1.0f, 0.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.0f, 0.18f, 0.8f, 0.7f, 1.0f, 1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, 0.0f, 0.8f, 0.1f, 0.96f, 1.0f, 1.0f, 1.0f, 0.0f,
        -0.5f,  0.5f, 0.0f, 0.18f, 0.5f, 0.6f, 1.0f, 0.0f, 1.0f, 0.0f,

        -0.5f, 1.0f, 0.0f, 1.0f, 0.94f, 0.24f, 1.0f, 0.0f, 0.0f, 1.0f, 
        -0.5f, 0.5f, 0.0f, 1.0f, 0.7f, 0.1f, 1.0f, 1.0f, 0.0f, 1.0f,
        -1.0f, 0.5f, 0.0f, 0.7f, 0.84f, 0.04f, 1.0f, 1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 0.0f, 1.0f, 0.94f, 0.24f, 1.0f, 0.0f, 1.0f, 1.0f
        };*/

        uint32_t indexCount = 0;
        vertex vertices[1000];
        vertex* buffer = vertices;
        for (int y = -6; y < 5; y++) {
            for (int x = -6; x < 5; x++) {
                buffer = CreateQuad(buffer, x * 100, y * -100, 100.0f, 100.0f, (x + y) % 2, WHITE);
                indexCount += 6;
            }
        }
    
        buffer = CreateQuad(buffer, playerPos.x, playerPos.y, 50.0f, 50.0f, 0, (Vec4) { 0.1f, 0.8f, 0.7f, 1.0f });  
        indexCount += 6;
        
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices[0]);

        glViewport(0, 0, width, height);
        
        mat4 projection = GLM_MAT4_IDENTITY_INIT;
        glm_ortho(0.0f, width, height, 0.0f, -1.0f, 1.0f, projection);
        glUniformMatrix4fv(glGetUniformLocation(program.shaderID, "projection"), 1, false, projection[0]);

        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArrayOES(vao);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
 
    }
 
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
