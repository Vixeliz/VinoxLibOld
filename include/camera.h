#ifndef CAMERA_H
#define CAMERA_H

#include "raymath.h"

typedef struct {
    float rotation;
    float scale;
    Vector2 position;
    Vector2 origin;
} Camera;

Matrix vinoxCameraMatrix(Camera *camera, Matrix projection);
Vector2 vinoxScreenToWorld(Vector2 position, Camera *camera);
Vector2 vinoxWorldToScreen(Vector2 position, Camera *camera, Vector2 screenSize);

#endif
