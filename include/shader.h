#ifndef VINOX_SHADER_H
#define VINOX_SHADER_H

typedef struct {
    unsigned int shaderID;
    unsigned int vertexShader;
    unsigned int fragmentShader;
    const char* vSrc;
    const char* fSrc;
} ShaderProgram;

int vinoxCompileShader(ShaderProgram *program);

#endif
