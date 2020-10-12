#define GLFW_NO_INCLUDE
#include <GLFW/glfw3.h>
#include <vinox/vinox.h>
#include <vinox/camera.h>
#include <vinox/texture.h>
#include <vinox/sound.h>
#include <vinox/buffer.h>
#include <stdbool.h>
#include <stdio.h>

#define SCROLLINCREASE 1
#define GRAVITY 1400

/* Function */
static int updatePlayer();
static int isKeyPressed(GLFWwindow *window, int keycode);
void processInput(GLFWwindow *window);

/* Global variables in file */
typedef struct {
    bool isDead;
    bool isGrounded;
    Quad quad;
    Vector2 velocity;
    int currentTexture;
} Dinosaur;

static Texture dinosaurDuckTex;
static Texture dinosaurTex;
static Dinosaur dino;
static int floorLimit;
static int scrollSpeed;
static float deltaTime = 0.0f;
static float lastFrame = 0.0f;
static const int gameWidth = 256;
static const int gameHeight = 144;

int updatePlayer() {
    dino.quad.position.y += dino.velocity.y * deltaTime;
    if (!dino.isGrounded) {
        dino.velocity.y += GRAVITY * deltaTime;
    }
    
    if (dino.quad.position.y >= floorLimit) {
        dino.isGrounded = true;
        dino.velocity.y = 0;
        dino.quad.position.y = floorLimit;
    }

    return 0;
}

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
    
    if ((isKeyPressed(window, GLFW_KEY_SPACE) || isKeyPressed(window, GLFW_KEY_W)) && dino.isGrounded) {

        dino.velocity.y = -400.0f;
        dino.isGrounded = false;
    }
    
    if (isKeyPressed(window, GLFW_KEY_DOWN) || isKeyPressed(window, GLFW_KEY_S)) {
        dino.velocity.y = 300.0f;
        dino.currentTexture = dinosaurDuckTex.id;

    } else {
        dino.currentTexture = dinosaurTex.id;
    }

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

    /* Load textures */
    FrameBuffer renderTexture;
    renderTexture.scaleType = GL_NEAREST;
    renderTexture.texture.width = gameWidth;
    renderTexture.texture.height = gameHeight;
    vinoxCreateFramebuffer(&renderTexture);

    vinoxLoadTexture("resources/dinosaur.png", &dinosaurTex, GL_NEAREST);
    vinoxLoadTexture("resources/dinosaurduck.png", &dinosaurDuckTex, GL_NEAREST);

    /* Ground quad */
    Quad floor;
    floor.size = (Vector2) { gameWidth, 32.0f };
    floor.position = (Vector2) { floor.size.x/2, gameHeight - floor.size.y/2 };
    floorLimit = gameHeight - floor.size.y - 16.0f;

    /* Set up player */
    dino.isDead = false;
    dino.isGrounded = true;
    dino.quad.size = (Vector2) { 32.0f, 32.0f };
    dino.quad.position = (Vector2) { 32.0f, floorLimit };
    dino.currentTexture = dinosaurTex.id;
    
    while (!glfwWindowShouldClose(window)) {
        int width, height;

        glfwGetFramebufferSize(window, &width, &height);
  
        processInput(window);
        
        updatePlayer();
        
        vinoxBeginDrawing(width, height);
            vinoxClear((Vector4){ 0.2f, 0.2f, 0.2f, 1.0f });
            vinoxBeginTexture(&renderTexture);
                vinoxClear((Vector4) { 0.0, 0.0, 0.0, 1.0});
                    vinoxCreateQuad(floor, EMPTYQUAD, 0, WHITE, 0.0f);
                    vinoxCreateQuad(dino.quad, EMPTYQUAD, dino.currentTexture, WHITE, 0.0f);
            vinoxEndTexture(&renderTexture);
            vinoxCreateQuad((Quad) { { width/2, height/2 }, { width, -height } }, EMPTYQUAD, renderTexture.texture.id, WHITE, 0.0f);
        vinoxEndDrawing();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}
