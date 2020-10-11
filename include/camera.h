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

#endif
