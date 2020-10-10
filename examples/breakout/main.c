#define GLFW_NO_INCLUDE
#include <GLFW/glfw3.h>
#include <vinox/vinox.h>
#include <vinox/camera.h>
#include <vinox/texture.h>
#include <vinox/sound.h>
#include <vinox/buffer.h>
#include <stdbool.h>
#include <stdio.h>
#include "stretchy_buffer.h"

#define PLAYERCOLOR (Vector4) { 0.0f, 1.0f, 0.0f, 1.0f }
#define RED (Vector4) { 1.0f, 0.0f, 0.0f, 1.0f }

Vector2 ballIntialVelocity = (Vector2) { 100.0f, -350.0f };
Vector2 UP = (Vector2) { 0.0f, 1.0f };
Vector2 RIGHT = (Vector2) { 1.0f, 0.0f };
Vector2 DOWN = (Vector2) { 0.0f, -1.0f };
Vector2 LEFT = (Vector2) {-1.0f, 0.0f };

typedef struct {
    bool didCollide;
    Vector2 direction;
    Vector2 resolution;
} collisionInfo;

typedef struct {
    Vector2 size;
    Vector2 position;
    float velocity;
} Paddle;

typedef struct {
    bool isDestroyed;
    bool isSolid;
    Vector2 position;
    Vector2 size;
    Vector4 color;
} Brick;

typedef struct {
    Vector2 position;
    Vector2 size;
    Vector2 velocity;
    float radius;
    bool stuck;
} Ball;

/* Functions */
static int loadLevel(const char *path);

/* Global variables in file */
static float deltaTime = 0.0f;
static float lastFrame = 0.0f;
static Ball ball;
static Paddle paddle;
static FILE *levelFile;
static Brick *bricks = NULL;
static int levelData[12][16] = {
{ 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 4, 2, 2, 2, 5, 2 },
{ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 },
{ 2, 2, 3, 2, 4, 3, 5, 3, 4, 3, 3, 4, 2, 3, 3, 4 },
{ 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 5, 4, 4, 3, 3, 2 },
{ 2, 2, 3, 2, 4, 4, 5, 5, 1, 5, 3, 2, 2, 4, 3, 4 },
{ 2, 2, 2, 2, 3, 3, 4, 4, 3, 5, 4, 4, 3, 3, 2, 2 },
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

Vector2 Vector2Direction(Vector2 target) {
    Vector2 compass[4] = {
    (Vector2) { 0.0f, 1.0f },
    (Vector2) {1.0f, 0.0f },
    (Vector2) {0.0f, -1.0f },
    (Vector2) {-1.0f, 0.0f }};
    
    float max = 0.0f;
    unsigned int bestMatch = -1;
    for (unsigned int i = 0; i < 4; i++)
    {
        float dotProduct = Vector2DotProduct(Vector2Normalize(target), compass[i]);
        if (dotProduct > max)
        {
            max = dotProduct;
            bestMatch = i;
        }

    }

    return (Vector2) { bestMatch, bestMatch };
}


collisionInfo checkCollisionPlayer(Paddle *brick) {

    Vector2 center = (Vector2) { ball.position.x + ball.radius, ball.position.y + ball.radius };
    Vector2 aabb_half_extents = (Vector2) {brick->size.x/2, brick->size.y/2};
    Vector2 aabb_center = (Vector2) {brick->position.x, brick->position.y};

    Vector2 difference = Vector2Subtract(center, aabb_center);
    Vector2 clamped = (Vector2) { Clamp(difference.x, -aabb_half_extents.x, aabb_half_extents.x), Clamp(difference.y, -aabb_half_extents.y, aabb_half_extents.y) };
    Vector2 closest = (Vector2) { aabb_center.x + clamped.x, aabb_center.y + clamped.y };
    
    difference = Vector2Subtract(closest, center);


    if (Vector2Length(difference) < ball.radius) {
        return (collisionInfo) { true, Vector2Direction(difference), difference };
    } else {
        return (collisionInfo) { false, (Vector2) { 0.0f, 1.0f }, (Vector2) { 0.0f, 0.0f } };
    }
}
collisionInfo checkCollision(Brick *brick) {

    Vector2 center = (Vector2) { ball.position.x + ball.radius, ball.position.y + ball.radius };
    Vector2 aabb_half_extents = (Vector2) {brick->size.x/2, brick->size.y/2};
    Vector2 aabb_center = (Vector2) {brick->position.x + aabb_half_extents.x, brick->position.y + aabb_half_extents.y };

    Vector2 difference = Vector2Subtract(center, aabb_center);
    Vector2 clamped = (Vector2) { Clamp(difference.x, -aabb_half_extents.x, aabb_half_extents.x), Clamp(difference.y, -aabb_half_extents.y, aabb_half_extents.y) };
    Vector2 closest = (Vector2) { aabb_center.x + clamped.x, aabb_center.y + clamped.y };
    
    difference = Vector2Subtract(closest, center);


    if (Vector2Length(difference) < ball.radius) {
        return (collisionInfo) { true, Vector2Direction(difference), difference };
    } else {
        return (collisionInfo) { false, (Vector2) { 0.0f, 1.0f }, (Vector2) { 0.0f, 0.0f } };
    }
}

int ballMove() {
    if (!ball.stuck) {
        ball.position.x += ball.velocity.x * deltaTime;
        ball.position.y += ball.velocity.y * deltaTime;

        if (ball.position.x <= 0.0f)
        {
            ball.velocity.x = -ball.velocity.x;
            ball.position.x = 0.0f;
        } else if (ball.position.x + ball.size.x/2 >= 1280)
        {
            ball.velocity.x = -ball.velocity.x;
            ball.position.x = 1280 - ball.size.x/2;
        }

        if (ball.position.y <= 0.0f) {
            ball.velocity.y = -ball.velocity.y;
            ball.position.y = 0.0f;
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
    
    float velocity = paddle.velocity * deltaTime;
    if (isKeyPressed(window, GLFW_KEY_A)) {
        paddle.position.x -= velocity;
    }
    if (isKeyPressed(window, GLFW_KEY_D)) {
        paddle.position.x += velocity;
    }
    if (isKeyPressed(window, GLFW_KEY_SPACE)) {
        ball.stuck = false;
    }
}

int loadLevel(const char *path) {

    //levelFile = fopen(path, "r");
    for (int y = 0; y < 12; ++y) {
        for (int x = 0; x < 16; ++x) {
            if (levelData[y][x] == 1) {
                Brick brick;
                brick.position.x = 80 * x;
                brick.position.y = 60 * y;
                brick.size = (Vector2) { 80, 60 };
                brick.color = WHITE;
                brick.isSolid = true;
                sb_push(bricks, brick);
            }else if (levelData[y][x] > 1) {
                Brick brick;
                if (levelData[y][x] == 2) {
                    brick.color = (Vector4) { 0.2f, 0.6f, 1.0f, 1.0f };
                    brick.size = (Vector2) { 80, 60 };
                    brick.isSolid = false;
                }else if (levelData[y][x] == 3) {
                    brick.color = (Vector4) { 0.0f, 0.7f, 0.0f, 1.0f };
                    brick.size = (Vector2) { 80, 60 };
                    brick.isSolid = false;
                }else if (levelData[y][x] == 4) {
                    brick.color = (Vector4) { 0.4f, 0.8f, 0.7f, 1.0f };
                    brick.size = (Vector2) { 80, 60 };
                    brick.isSolid = false;
                }else if (levelData[y][x] == 5) {
                    brick.color = (Vector4) { 0.5f, 0.0f, 0.7f, 1.0f };
                    brick.size = (Vector2) { 80, 60 };
                    brick.isSolid = false;
                }
                brick.position.x = 80 * x;
                brick.position.y = 60 * y;
                sb_push(bricks, brick);
            }
        }
    }
    return 0;
}

int main(int argc, char *argv[]) {
    GLFWwindow* window;
    glfwInit();
    
    window = glfwCreateWindow(1280, 720, "Breakout", NULL, NULL);
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (vinoxInit(1280, 720) == -1) {
        printf("Init of vinox failed. aborting\n");
        return -1;
    }
    
    paddle.size = (Vector2) { 100.0f, 20.0f };
    paddle.velocity = 500.0f;
    paddle.position = (Vector2) { 1280/2, 720 - paddle.size.y }; 

    Camera camera;
    camera.scale = 1.0f;
    camera.rotation = 0.0f;
    camera.offsetX = 0.0f;
    camera.offsetY = 0.0f;
    camera.x = 0.0f;
    camera.y = 0.0f;

    ball.size = (Vector2) { 25.0f, 25.0f };
    ball.velocity = (Vector2) { 100.0f, -350.0f };
    ball.radius = 12.5f;
    ball.stuck = true;

    FrameBuffer renderTexture;
    renderTexture.texture.width = 1280;
    renderTexture.texture.height = 720;
    Texture brickTex;
    Texture solidTex;
    Texture backgroundTex;
    Texture paddleTex;
    Texture ballTex;
    vinoxLoadTexture("awesomeface.png", &ballTex);
    vinoxLoadTexture("paddle.png", &paddleTex);
    vinoxLoadTexture("block.png", &brickTex);
    vinoxLoadTexture("background.jpg", &backgroundTex);
    vinoxLoadTexture("block_solid.png", &solidTex);
    vinoxCreateFramebuffer(&renderTexture);
    loadLevel("");

            int rotation = 0;
            bool isLeft = true;
    while (!glfwWindowShouldClose(window)) {
        int width, height;

        if (rotation <= 0)
            isLeft = true;
        if (rotation >= 360)
            isLeft = false;
        
        if (isLeft == true)
            rotation++;

        if (isLeft == false)
            rotation--;
        
        processInput(window);
            
        if (paddle.position.x < 0 + paddle.size.x/2)
            paddle.position.x = 0 + paddle.size.x/2;

        if (paddle.position.x > 1280 - paddle.size.x/2)
            paddle.position.x = 1280 - paddle.size.x/2;
        if (ball.stuck)
            ball.position = (Vector2) { paddle.position.x, paddle.position.y - ball.size.y };
        
        ballMove();
        for (int i = 0; i < sb_count(bricks); i++) {
            if (!bricks[i].isDestroyed) {
                collisionInfo colInfo = checkCollision(&bricks[i]);
                if(colInfo.didCollide) {
                    if (!bricks[i].isSolid)
                        bricks[i].isDestroyed = true;
                    
                    Vector2 direction = colInfo.direction;
                    Vector2 diffVector = colInfo.resolution;
                    if ((direction.x == LEFT.x && direction.y == LEFT.y) || (direction.x == RIGHT.x && direction.y == RIGHT.y)) {
                        ball.velocity.x = -ball.velocity.x;

                        float penetration = ball.radius - abs(diffVector.x);
                        if (direction.x == LEFT.x && direction.y == LEFT.y)
                            ball.position.x += penetration;
                        else
                            ball.position.x -= penetration;

                    }else {
                        ball.velocity.y = -ball.velocity.y;

                        float penetration = ball.radius - abs(diffVector.y);
                        if (direction.x == UP.x && direction.y == UP.y)
                            ball.position.y -= penetration;
                        else
                            ball.position.y += penetration;
                    }
                    }
                }
            }
            collisionInfo colResult = checkCollisionPlayer(&paddle);
            if (ball.stuck == false && colResult.didCollide == true) {
                float distance = (ball.position.x + ball.radius) - paddle.position.x;
                float percentage = distance / (paddle.size.x / 2.0f);

                float strength = 2.0f;
                Vector2 oldVelocity = ball.velocity;
                ball.velocity.x = ballIntialVelocity.x * percentage * strength;
                ball.velocity.y = -1.0f * abs(ball.velocity.y);
                //Vector2 length = (Vector2) { Vector2Length(oldVelocity), Vector2Length(oldVelocity) };
                //ball.velocity = Vector2Normalize(Vector2Multiply(ball.velocity, length));
            }
        glfwGetFramebufferSize(window, &width, &height);

        vinoxBeginDrawing(width, height);
           vinoxClear((Vector4){ 0.2f, 0.2f, 0.2f, 1.0f });
            vinoxBeginTexture(&renderTexture);
                vinoxClear((Vector4) { 0.0, 0.0, 0.0, 1.0});
            vinoxBeginCamera(&camera);    
            vinoxCreateQuad(640, 360, 1280, 720, backgroundTex.id, WHITE, 0.0f);
        
            for (int i = 0; i < sb_count(bricks); i++) {
                if (!bricks[i].isDestroyed)
                    vinoxCreateQuad(bricks[i].position.x + 40, bricks[i].position.y + 30, 80, 60, brickTex.id, bricks[i].color, 0.0f);
            }   
                vinoxCreateQuad(paddle.position.x, paddle.position.y, paddle.size.x, paddle.size.y, paddleTex.id, WHITE, 0.0f);
                vinoxCreateQuad(ball.position.x, ball.position.y, ball.size.x, ball.size.y, ballTex.id, WHITE, rotation);
            vinoxEndCamera();
            vinoxEndTexture(&renderTexture);
                vinoxCreateQuad(width/2, height/2, width, -height, renderTexture.texture.id, WHITE, 0.0f);
        vinoxEndDrawing();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}
