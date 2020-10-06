#define GLFW_NO_INCLUDE
#include <GLFW/glfw3.h>
#define GL_GLEXT_PROTOTYPES 1
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include "vingl.h"

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
    camera.scale = 2.0f;
    camera.rotation = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        int width, height;

        glfwGetFramebufferSize(window, &width, &height);
        
        vinoxBeginDrawing(camera, width, height);
        glViewport(0, 0, width, height);
        vinoxEndDrawing();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}
