#define GLFW_NO_INCLUDE
#include <GLFW/glfw3.h>
#include <vinox/vinox.h>

/* Global variables in file */
float deltaTime = 0.0f;
float lastFrame = 0.0f;
vec2 playerPos;

void processInput(GLFWwindow *window) {

  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  float currentFrame = glfwGetTime();
  deltaTime = currentFrame - lastFrame;
  lastFrame = currentFrame;
  const float playerSpeed = 150.0f * deltaTime;
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    playerPos[2] -= playerSpeed;
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    playerPos[2] += playerSpeed;
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    playerPos[1] -= playerSpeed;
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    playerPos[1] += playerSpeed;
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

    playerPos[1] = 500.0f;
    playerPos[2] = 500.0f;
    
    Camera camera;
    camera.position;
    camera.position[1] = playerPos[1];
    camera.position[2] = playerPos[2];
    camera.scale = 2.0f;
    camera.rotation = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        int width, height;

        processInput(window);

        glfwGetFramebufferSize(window, &width, &height);
        
        camera.position[1] = playerPos[1];
        camera.position[2] = playerPos[2];
        
        vinoxBeginDrawing(camera, width, height);
           for (int y = -6; y < 5; y++) {
            for (int x = -6; x < 5; x++) {
            vinoxCreateQuad(x * 100.0f, y * 100.0f, 100.0f, 100.0f, 1, WHITE);
            }   
        }
            vinoxCreateQuad(playerPos[1], playerPos[2], 50.0f, 50.0f, 0, (vec4) { 0.1f, 0.6f, 0.5f, 1.0f });
        vinoxEndDrawing();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}
