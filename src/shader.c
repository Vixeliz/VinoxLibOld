#define EGL_NO_X11
#include <EGL/egl.h>
#define GL_GLEXT_PROTOTYPES 1
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <stddef.h>
#include <stdio.h>
#include "shader.h"

static int compileVertexShader(ShaderProgram *program) {
    const char *screenVSrc =
        "#version 320 es\n"
        "precision mediump float;\n"
        "layout (location = 0) in vec2 aPos;\n"
        "layout (location = 1) in vec2 aTexCoords;\n"
        "out vec2 TexCoords;\n"
        "void main()\n"
        "{\n"
        "   TexCoords = aTexCoords;\n"
        "   gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);\n"
        "}\n";
    
    const char* vertexSource =                                                    
        "#version 320 es\n"
        "precision mediump float;\n"
        "layout (location = 0) in vec3 aPos;\n"                                     
        "layout (location = 1) in vec4 aColor;\n" 
        "layout (location = 2) in vec2 aTexCoord;\n" 
        "layout (location = 3) in float aTexIndex;\n" 
        "out vec4 vColor;\n"
        "out vec2 vTexCoord;\n"
        "out float vTexIndex;\n"
        "uniform mat4 projection;\n"
        "void main()\n"                                           
        "{\n"               
        "   vColor = aColor;\n"
        "   vTexCoord = aTexCoord;\n"
        "   vTexIndex = aTexIndex;\n"
        "   gl_Position = projection * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"                               "}\n\0";
    /* Based off of what type of program we wanted assign one of the two default
     * shaders */
    const char* currentSource;
    if (program->type == 0) {
        currentSource = vertexSource;
    } else if (program->type == 1) {
        currentSource = screenVSrc;
    } else {
        printf("No shader type\n");
    }
    
    /* Compile the shader */
    program->vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(program->vertexShader, 1, &currentSource, NULL);
    glCompileShader(program->vertexShader);
    
    return 0;
}

static int compileFragmentShader(ShaderProgram *program) {

    const char *screenFSrc =
        "#version 320 es\n"
        "precision mediump float;\n"
        "layout (location = 0) out vec4 FragColor;\n"
        "in vec2 TexCoords;\n"
        "uniform sampler2D screenTexture;\n"
        "void main()\n"
        "{\n"
        "   vec3 col = texture(screenTexture, TexCoords).rgb;\n"
        "   FragColor = vec4(col, 1.0);\n"
        "}\n";

    const char* fragmentSource = 
        "#version 320 es\n"
        "precision mediump float;\n"
        "in vec4 vColor;\n"
        "in vec2 vTexCoord;\n"
        "in float vTexIndex;\n"
        "layout (location = 0) out vec4 FragColor;\n"
        "uniform sampler2D uTextures[8];\n"
        "void main()\n"
        "{\n"
        "   int index = int(vTexIndex);\n"
        "   if (index != 0) {\n"
        "   FragColor = vColor * texture(uTextures[index], vTexCoord);\n"
        "   } else {\n"
        "   FragColor = vColor;\n"
        "   }\n"
        "}\n\0";
    
    /* Based off of what type of program we wanted assign one of the two default
     * shaders */
    const char* currentSource;
    if (program->type == 0) {
        currentSource = fragmentSource;
    } else if (program->type == 1) {
        currentSource = screenFSrc;
    } else {
        printf("No shader type\n");
    }

    /* Compile the shader */
    program->fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(program->fragmentShader, 1, &currentSource, NULL);
    glCompileShader(program->fragmentShader);

    return 0;
}

int vinoxCompileShader(ShaderProgram *program) {
int abort = 0;
int testval;

    compileVertexShader(program);
    compileFragmentShader(program);
    
    /* Link the shaders to the given program */
    program->shaderID = glCreateProgram();
    glAttachShader(program->shaderID, program->vertexShader);
    glAttachShader(program->shaderID, program->fragmentShader);
    glLinkProgram(program->shaderID);

    /* Make sure no errors are present */
    glGetShaderiv(program->vertexShader, GL_COMPILE_STATUS, &testval);
    if(testval == GL_FALSE) {
        char infolog[1024];
        glGetShaderInfoLog(program->vertexShader, 1024, NULL, infolog);
        printf(infolog);
	    abort = -1;
	    printf("The vertex shader failed to compile\n");
    }

    glGetShaderiv(program->fragmentShader, GL_COMPILE_STATUS, &testval);
    if(testval == GL_FALSE) {
        char infolog[1024];
        glGetShaderInfoLog(program->fragmentShader, 1024, NULL, infolog);
        printf(infolog);
	    abort = -1;
	    printf("The fragment shader failed to compile\n");
    }

    glGetProgramiv(program->shaderID, GL_LINK_STATUS, &testval);
    if(testval == GL_FALSE) {
	    abort = -1;
	    printf("The shaderProgram failed to link\n");
    }

    return abort;
}
