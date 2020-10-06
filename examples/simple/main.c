#define GLFW_NO_INCLUDE
#include <GLFW/glfw3.h>
#include <vinox/vinox.h>

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
    camera.position = (Vec2) { 500.0f, 500.0f };
    camera.scale = 1.0f;
    camera.rotation = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        int width, height;

        glfwGetFramebufferSize(window, &width, &height);
        
        vinoxBeginDrawing(camera, width, height);
           for (int y = -6; y < 5; y++) {
            for (int x = -6; x < 5; x++) {
            vinoxCreateQuad(x * 100.0f, y * 100.0f, 100.0f, 100.0f, 1, WHITE);
            }   
        }
        vinoxEndDrawing();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}
