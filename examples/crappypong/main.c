#define GLFW_NO_INCLUDE
#include <GLFW/glfw3.h>
#include <vinox/vinox.h>
#include <stdio.h>
#include <stdbool.h>

/* Structures */
typedef struct {
    Vector2 pos;
    Vector2 size;
    Vector2 mov;
} Ball;

typedef struct {
    Vector2 pos;
    Vector2 size;
} Paddle;

/* Global variables in file */
static Ball ball;
static Paddle paddle[2];
static int scores[2];
static float deltaTime = 0.0f;
static float lastFrame = 0.0f;

static int resetBall() {
    ball.pos = (Vector2) { 1920/2, 1080/2 };
    ball.size = (Vector2) { 25, 25 };
    ball.mov = (Vector2) { 7, 0 };

    return 0;
}

static int checkCollision(Paddle paddle) {
    
    int leftB, leftP;
    int rightB, rightP;
    int topB, topP;
    int bottomB, bottomP;

    leftB = ball.pos.x;
    rightB = ball.pos.x + ball.size.x;
    topB = ball.pos.y;
    bottomB = ball.pos.y + ball.size.y;

    leftP = paddle.pos.x;
    rightP = paddle.pos.x + paddle.size.x;
    topP = paddle.pos.y;
    bottomP = paddle.pos.y + paddle.size.y;

    if (leftB > rightP)
        return 0;

    if (rightB < leftP)
        return 0;

    if (topB > bottomP)
        return 0;

    if (bottomB < topP)
        return 0;

    return 1;
}

static int printScore() {
    printf("----------------------------\n");
    printf("Player One Score: %i\n", scores[0]);
    printf("Player Two Score: %i\n", scores[1]);
    printf("----------------------------\n");

    return 0;
}

static int moveBall() {
    
    ball.pos.x += ball.mov.x;
    ball.pos.y += ball.mov.y;

    if (ball.pos.x < 0) {
        scores[1] += 1;
        printScore();
        resetBall();
    }

    if (ball.pos.x > 1920 - ball.size.x) {
        scores[0] += 1;
        printScore();
        resetBall();
    }

    if (ball.pos.y < 0 || ball.pos.y > 1080 - ball.size.y)
        ball.mov.y = -ball.mov.y;

    for (int i = 0; i < 2; i++) {
        if (checkCollision(paddle[i])) {
            if (ball.mov.x < 0)
                ball.mov.x -= 1;
            else
                ball.mov.x += 1;

            ball.mov.x = -ball.mov.x;

            int hitPos = (paddle[i].pos.y + paddle[i].size.y) - ball.pos.y;
            
            if (hitPos >= 0 && hitPos < (paddle[i].size.y * 0.1)) {
                ball.mov.y = 7;
            }
            else if (hitPos >= (paddle[i].size.y * 0.1) && hitPos < (paddle[i].size.y * 0.2)) {
                ball.mov.y = 6;
            }
            else if (hitPos >= (paddle[i].size.y * 0.2) && hitPos < (paddle[i].size.y * 0.3)) {
                ball.mov.y = 5;
            }
            else if (hitPos >= (paddle[i].size.y * 0.3) && hitPos < (paddle[i].size.y * 0.4)) {
                ball.mov.y = 4;
            }
            else if (hitPos >= (paddle[i].size.y * 0.4) && hitPos < (paddle[i].size.y * 0.5)) {
                ball.mov.y = 0;
            }
            else if (hitPos >= (paddle[i].size.y * 0.5) && hitPos < (paddle[i].size.y * 0.6)) {
                ball.mov.y = -4;
            }
            else if (hitPos >= (paddle[i].size.y * 0.6) && hitPos < (paddle[i].size.y * 0.7)) {
                ball.mov.y = -5;
            }
            else if (hitPos >= (paddle[i].size.y * 0.7) && hitPos < (paddle[i].size.y * 0.8)) {
                ball.mov.y = -6;
            }
            else if (hitPos >= (paddle[i].size.y * 0.8) && hitPos < (paddle[i].size.y * 0.9)) {
                ball.mov.y = -7;
            } else {
                ball.mov.y = -7.5;
            }


            if (ball.mov.x > 0) {

                if (ball.pos.x < 30)
                    ball.pos.x = 30;
            } else {
                if (ball.pos.x > 1840)
                    ball.pos.x = 1840;
        }
        }
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
    
    const float playerSpeed = 750.0f * deltaTime;
    if (isKeyPressed(window, GLFW_KEY_W)) {
        paddle[0].pos.y -= playerSpeed;
    }
    if (isKeyPressed(window, GLFW_KEY_S)) {
        paddle[0].pos.y += playerSpeed;
    }
    
    if (isKeyPressed(window, GLFW_KEY_UP)) {
        paddle[1].pos.y -= playerSpeed;
    }
    if (isKeyPressed(window, GLFW_KEY_DOWN)) {
        paddle[1].pos.y += playerSpeed;
    }
}

int main(void) {
    GLFWwindow* window;
    glfwInit();
    
    window = glfwCreateWindow(1920, 1080, "Vinox", NULL, NULL);
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (vinoxInit(1920, 1080) == -1) {
        printf("Init of vinox failed. aborting\n");
        return -1;
    }

    Camera camera;
    camera.x = 0.0f;
    camera.y = 0.0f;
    camera.scale = 1.0f;
    camera.rotation = 0.0f;

    resetBall();

    paddle[0].size = (Vector2) { 25.0f, 150.0f };
    paddle[1].size = (Vector2) { 25.0f, 150.0f };
    paddle[0].pos = (Vector2) { 25.0f, 0.0f };
    paddle[1].pos= (Vector2) { 1920.0f - paddle[1].size.x - 25.0f, 0.0f };
    
    while (!glfwWindowShouldClose(window)) {
        int width, height;

        processInput(window);
        
        if (!(paddle[0].pos.y <= 1080 - paddle[0].size.y))
            paddle[0].pos.y = 1080 - paddle[0].size.y;

        if (!(paddle[0].pos.y >= 0))
            paddle[0].pos.y = 0;

        if (!(paddle[1].pos.y <= 1080 - paddle[1].size.y))
            paddle[1].pos.y = 1080 - paddle[1].size.y;

        if (!(paddle[1].pos.y >= 0))
            paddle[1].pos.y = 0;
        
        moveBall();

        glfwGetFramebufferSize(window, &width, &height);
        
        camera.offsetX = 0;
        camera.offsetY = 0;

        vinoxBeginDrawing(camera, width, height);
        vinoxCreateQuad(0.0f, 0.0f, 15, 1080, 0, WHITE);
        vinoxCreateQuad(1920.0f - 15, 0.0f, 15, 1080, 0, WHITE);
        vinoxCreateQuad(paddle[0].pos.x, paddle[0].pos.y, paddle[0].size.x, paddle[0].size.y, 0, (Vector4) { 0.1f, 0.6f, 0.5f, 1.0f });
        vinoxCreateQuad(paddle[1].pos.x, paddle[1].pos.y, paddle[1].size.x, paddle[1].size.y, 0, (Vector4) { 0.1f, 0.6f, 0.5f, 1.0f });
        vinoxCreateQuad(ball.pos.x, ball.pos.y, ball.size.x, ball.size.y, 0, WHITE);
        vinoxEndDrawing();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}
