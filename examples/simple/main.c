#define GLFW_NO_INCLUDE
#include <GLFW/glfw3.h>
#include <vinox/vinox.h>

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

    if (vinoxInit() == -1) {
        printf("Init of vinox failed. aborting\n");
        return -1;
    }

    Camera camera;
    camera.scale = 2.0f;
    camera.rotation = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        int width, height;

        processInput(window);

        glfwGetFramebufferSize(window, &width, &height);
        
        camera.position.x = playerPos.x;
        camera.position.y = playerPos.y;
        camera.origin.x = width/2 - 50.0f;
        camera.origin.y = height/2 - 50.0f;

        vinoxBeginDrawing(camera, width, height);
           for (int y = 0; y < 100; y++) {
            for (int x = 0; x < 100; x++) {
                vinoxCreateQuad(x * 10.0f, y * 10.0f, 10.0f, 10.0f, 1, WHITE);
            }   
        }
            vinoxCreateQuad(playerPos.x, playerPos.y, 50.0f, 50.0f, 0, (vec4) { 0.1f, 0.6f, 0.5f, 1.0f });
        vinoxEndDrawing();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}
