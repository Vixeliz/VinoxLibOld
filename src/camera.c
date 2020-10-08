#include "camera.h"

int vinoxCameraMatrix(mat4 viewproj, Camera *camera, int width, int height) {
    /* Camera transformations */
    mat4 projection = GLM_MAT4_IDENTITY_INIT;
    mat4 view = GLM_MAT4_IDENTITY_INIT;
    glm_ortho(0.0f, width, height, 0.0f, -1.0f, 1.0f, projection);
        
    /* Camera origin */
    mat4 position = GLM_MAT4_IDENTITY_INIT;
    glm_translate(position, (vec3) { camera->offsetX, camera->offsetY, 0.0f });
        
    /* Camera rotation */
    glm_rotate(position, glm_rad(camera->rotation), (vec3) { 0.0f, 0.0f, 1.0f });
        
    /* Camera zoom */
    glm_scale(position, (vec3) { camera->scale, camera->scale, 1.0f });
        
    /* Camera position */
    vec3 camPosition = { -camera->x, -camera->y, 0.0f };
    glm_translate(position, camPosition);
        
        
    //glm_mat4_mul(position, rotation, view);
    glm_mat4_copy(position, view);
    glm_mat4_mul(projection, view, viewproj);
    
    return 0;
}
