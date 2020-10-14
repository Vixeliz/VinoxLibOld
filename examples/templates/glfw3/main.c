#define GLFW_NO_INCLUDE
#include <GLFW/glfw3.h>
#include <vinox/vinox.h>
#include <stdbool.h>
#include <stdio.h>

/* MACROS */
#define SCREENWIDTH 640
#define SCREENHEIGHT 480

/* Functions */
int isKeyPressed(GLFWwindow *window, int keycode);
void processInput(GLFWwindow *window);

/* Global variables */
/* Struct to hold to all global variables to keep code clean */
typedef struct {
    float deltaTime;
    float lastFrame;
    int frameCounter;
    GLFWwindow *window;
} State;

static State state;

int main(void) {
    /* Init glfw, create window and gl context */
    glfwInit();

    state.window = glfwCreateWindow(SCREENWIDTH, SCREENHEIGHT, "Vinox - glfw example", NULL, NULL);

    glfwMakeContextCurrent(state.window);
    glfwSwapInterval(0);

    /* Set default variables for our state */
    state.deltaTime = 0.0f;
    state.lastFrame = 0.0f;
    state.frameCounter = 0;

    /* Initialize vinox before doing anything vinox related */
    vinoxInit(SCREENWIDTH, SCREENHEIGHT);

    /* Program loop */
    state.lastFrame = glfwGetTime();
    while (!glfwWindowShouldClose(state.window)) {

        /* Get the current window size */
        int windowWidth, windowHeight;
        glfwGetFramebufferSize(state.window, &windowWidth, &windowHeight);

        /* Calculate FPS */
        float currentTime = glfwGetTime();
        state.frameCounter++;

        if (currentTime - state.lastFrame >= 1.0) {
            printf("FPS: %i\n", state.frameCounter);
            printf("ms time: %f\n", 1000.0/(double)(state.frameCounter));

            state.frameCounter = 0;
            state.lastFrame = currentTime;
        }
        
        /* Process input */
        processInput(state.window);
        
        double mousePosx, mousePosy;
        glfwGetCursorPos(state.window, &mousePosx, &mousePosy); 
        Vector2 mousePos = (Vector2) { (int)mousePosx, (int)mousePosy };

        /* Draw */
        vinoxBeginDrawing(windowWidth, windowHeight);
            vinoxClear(WHITE);
            vinoxCreateQuad((Quad) { { windowWidth/2, windowHeight/2 }, { 50.0f, 50.0f } }, EMPTYQUAD, 0, GREEN, 0.0f);
        vinoxEndDrawing();

        /* Switch buffers and poll events */
        glfwSwapBuffers(state.window);
        glfwPollEvents();
    }

    vinoxEnd();
    return 0;
}

int isKeyPressed(GLFWwindow *window, int keycode) {
    
    int state = glfwGetKey(window, keycode);
    return state == GLFW_PRESS || state == GLFW_REPEAT;
}

void processInput(GLFWwindow *window) {

    if (isKeyPressed(window, GLFW_KEY_ESCAPE))
            glfwSetWindowShouldClose(window, true);

}
