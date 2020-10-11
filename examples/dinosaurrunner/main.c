#define GLFW_NO_INCLUDE
#include <GLFW/glfw3.h>
#include <vinox/vinox.h>
#include <vinox/camera.h>
#include <vinox/texture.h>
#include <vinox/sound.h>
#include <vinox/buffer.h>
#include <stdbool.h>
#include <stdio.h>

/* Global variables in file */
static float deltaTime = 0.0f;
static float lastFrame = 0.0f;
static const int gameWidth = 256;
static const int gameHeight = 144;

static int isKeyPressed(GLFWwindow *window, int keycode);
void processInput(GLFWwindow *window);

int isKeyPressed(GLFWwindow *window, int keycode) {
    int state = glfwGetKey(window, keycode);
    return state == GLFW_PRESS || state == GLFW_REPEAT;
}

void processInput(GLFWwindow *window) {
    

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    
    const float playerSpeed = 150.0f * deltaTime;
}

int main(void) {
    GLFWwindow* window;
    glfwInit();
    
    window = glfwCreateWindow(1280, 720, "Vinox", NULL, NULL);
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (vinoxInit(gameWidth, gameHeight) == -1) {
        printf("Init of vinox failed. aborting\n");
        return -1;
    }

    /* Set up camera */
    Camera camera;
    camera.scale = 1.0f;
    camera.rotation = 0.0f;
    camera.origin = (Vector2) { gameWidth/2, gameHeight/2 };

    /* Load textures */
    FrameBuffer renderTexture;
    renderTexture.texture.width = gameWidth;
    renderTexture.texture.height = gameHeight;
    vinoxCreateFramebuffer(&renderTexture);

    Texture dinosaurTex;
    vinoxLoadTexture("dinosaur.png", &dinosaurTex, GL_NEAREST);

    while (!glfwWindowShouldClose(window)) {
        int width, height;

        processInput(window);
    
        glfwGetFramebufferSize(window, &width, &height);
        
        camera.position = (Vector2) { 0.0f, 0.0f };

        vinoxBeginDrawing(width, height);
            vinoxClear((Vector4){ 0.2f, 0.2f, 0.2f, 1.0f });
            vinoxBeginTexture(&renderTexture);
                vinoxClear((Vector4) { 0.0, 0.0, 0.0, 1.0});
                vinoxBeginCamera(&camera);    
                    vinoxCreateQuad((Quad){{0.0f, 0.0f}, { 32, 32}}, EMPTYQUAD, dinosaurTex.id, WHITE, 0.0f);
                vinoxEndCamera();
            vinoxEndTexture(&renderTexture);
            vinoxCreateQuad((Quad) { {width/2, height/2}, {width, -height} }, EMPTYQUAD, renderTexture.texture.id, WHITE, 0.0f);
        vinoxEndDrawing();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}
