#ifndef VINOX_SHADER_H
#define VINOX_SHADER_H

typedef struct {
    unsigned int shaderID;
    unsigned int vertexShader;
    unsigned int fragmentShader;
    unsigned int type;
} ShaderProgram;

int vinoxCompileShader(ShaderProgram *program);

#endif
