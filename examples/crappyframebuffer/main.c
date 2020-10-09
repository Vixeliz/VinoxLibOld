#define GLFW_NO_INCLUDE
#include <GLFW/glfw3.h>
#include <vinox/vinox.h>
#include <vinox/camera.h>
#include <vinox/texture.h>
#include <vinox/sound.h>
#include <vinox/buffer.h>
#include <stdbool.h>
#include <stdio.h>

#define max(a, b) ((a)>(b)? (a) : (b))
#define min(a, b) ((a)<(b)? (a) : (b))

#define PLAYERCOLOR (Vector4) { 0.0f, 1.0f, 0.0f, 1.0f }
#define RED (Vector4) { 1.0f, 0.0f, 0.0f, 1.0f }

/* Global variables in file */
static float deltaTime = 0.0f;
static float lastFrame = 0.0f;
static Vector2 playerPos = (Vector2) { 0.0f, 0.0f };

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
    if (isKeyPressed(window, GLFW_KEY_W)) {
        playerPos.y -= playerSpeed;
    }
    if (isKeyPressed(window, GLFW_KEY_S)) {
        playerPos.y += playerSpeed;
    }
    if (isKeyPressed(window, GLFW_KEY_A)) {
        playerPos.x -= playerSpeed;
    }
    if (isKeyPressed(window, GLFW_KEY_D)) {
        playerPos.x += playerSpeed;
    }
}

int main(void) {
    GLFWwindow* window;
    glfwInit();
    
    window = glfwCreateWindow(640, 480, "Vinox", NULL, NULL);
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (vinoxInit(640, 480) == -1) {
        printf("Init of vinox failed. aborting\n");
        return -1;
    }

    Camera camera;
    camera.scale = 1.0f;
    camera.rotation = 0.0f;

    FrameBuffer renderTexture;
    renderTexture.texture.width = 640;
    renderTexture.texture.height = 480;
    Texture containerTex;
    Texture smileTex;
    vinoxLoadTexture("test.jpg", &containerTex);
    vinoxLoadTexture("awesomeface.png", &smileTex);
    vinoxCreateFramebuffer(&renderTexture);

    if(vinoxPlaySound("~/test.mp3") == -1)
        printf("Sound failed to play!\n");

    while (!glfwWindowShouldClose(window)) {
        int width, height;

        processInput(window);
    
        if (playerPos.x < 0)
            playerPos.x = 0;

        if (playerPos.x > 1000)
            playerPos.x = 1000;
        
        if (playerPos.y < 0)
            playerPos.y = 0;
        
        if (playerPos.y > 1000)
            playerPos.y = 1000;

        glfwGetFramebufferSize(window, &width, &height);
        float scale = min((float)width/640, (float)height/480);
        
        camera.x = playerPos.x;
        camera.y = playerPos.y;
        camera.offsetX = 640/2;
        camera.offsetY = 480/2;

        vinoxBeginDrawing(width, height);
           vinoxClear((Vector4){ 0.2f, 0.2f, 0.2f, 1.0f });
            vinoxBeginTexture(&renderTexture);
                vinoxClear((Vector4) { 0.0, 0.0, 0.0, 1.0});
            vinoxBeginCamera(&camera);    
            for (int y = 0; y < 10; y++) {
            for (int x = 0; x < 10; x++) {
                int id = 0;
                if ((x + y) % 2 == 0)
                    id = containerTex.id;
                else if ((x + y) % 2 == 1)
                    id = smileTex.id;
                
                vinoxCreateQuad((x * 100.0f) + 50.0f, (y * 100.0f) + 50.0f, 100.0f, 100.0f, id, WHITE, 180.0f);
            }   
        }
                vinoxCreateQuad(playerPos.x, playerPos.y, 50, 50, 0, PLAYERCOLOR, 45.0f);
            vinoxEndCamera();
            vinoxEndTexture(&renderTexture);
                vinoxCreateQuad(width/2, height/2, width, -height, renderTexture.texture.id, WHITE, 0.0f);
        vinoxEndDrawing();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}
