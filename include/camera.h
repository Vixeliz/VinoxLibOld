#ifndef CAMERA_H
#define CAMERA_H

#include "raymath.h"

typedef struct {
    float rotation;
    float scale;
    int x, y;
    int offsetX, offsetY;
} Camera;

Matrix vinoxCameraMatrix(Camera *camera, Matrix projection);

#endif
