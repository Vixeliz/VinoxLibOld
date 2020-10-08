#include "camera.h"

Matrix vinoxCameraMatrix(Camera *camera, int width, int height) {
    /* Camera transformations */
    Matrix projection = MatrixIdentity();
    Matrix view = MatrixIdentity();
    Matrix viewproj = MatrixIdentity();
    Matrix ortho = MatrixOrtho(0.0f, width, height, 0.0f, -1.0f, 1.0f);
    projection = MatrixMultiply(projection, ortho);

    /* Camera origin */
    Matrix origin = MatrixTranslate(camera->offsetX, camera->offsetY, 0.0f);
    view = MatrixMultiply(origin, view);

        
    /* Camera rotation */
    Matrix rotate = MatrixIdentity();
    Vector3 axis = (Vector3) { 0.0f, 0.0f, 1.0f };
    rotate = MatrixRotate(Vector3Normalize(axis), camera->rotation*DEG2RAD);
    view = MatrixMultiply(rotate, view);    
    
    /* Camera zoom */
    Matrix scale = MatrixScale(camera->scale, camera->scale, 1.0f);
    view = MatrixMultiply(scale, view);
        
    /* Camera position */
    Matrix position = MatrixTranslate(-camera->x, -camera->y, 0.0f);
    view = MatrixMultiply(position, view);
     
    viewproj = MatrixMultiply(view, projection);
    
    return viewproj;
}
