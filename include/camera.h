#ifndef CAMERA_H
#define CAMERA_H

#include <cglm/cglm.h>

typedef struct {
    float rotation;
    float scale;
    int x, y;
    int offsetX, offsetY;
} Camera;

int vinoxCameraMatrix(mat4 viewproj, Camera *camera, int width, int height);

#endif
