//////////////////////////////////////////////////////////////////////////////
//
//  init_shader.cc
//
//  The source file that offers an easy way to init vertex and fragment shaders.
//
//  Project         : SmfView
//  Name            : Chong Guo
//  Student ID      : 301295753
//  SFU username    : armourg
//  Instructor      : Richard Zhang
//
//  Created by Armour on 2/11/2016
//  Copyright (c) 2016 Armour. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////////

#include "init_shader.h"
#include <iostream>

namespace angel {

//
// Function: ReadShaderSource
// ---------------------------
//
//   Create a NULL-terminated string by reading the provided file
//
//   Parameters:
//       shader_file: the name of the file that need to read
//
//   Returns:
//       the content of shaderFile
//

char *ReadShaderSource(const char *shader_file) {
    FILE *fp = fopen(shader_file, "r");
    if (fp == NULL) { return NULL; }

    fseek(fp, 0L, SEEK_END);
    long size = ftell(fp);

    fseek(fp, 0L, SEEK_SET);
    char *buf = new char[size + 1];
    fread(buf, 1, size, fp);

    buf[size] = '\0';
    fclose(fp);
    return buf;
}

//
// Function: InitShader
// ---------------------------
//
//   Create a GLSL program object from vertex and fragment shader files
//
//   Parameters:
//       v_shader_file: the name of the vertex shader file that need to read
//       f_shader_file: the name of the fragment shader file that need to read
//
//   Returns:
//       the binded id of GLSL program object
//

GLuint InitShader(const char *v_shader_file, const char *f_shader_file) {
    struct Shader {
        const char *filename_;
        GLenum type_;
        GLchar *source_;
    } shaders[2] = {
        { v_shader_file, GL_VERTEX_SHADER, NULL },
        { f_shader_file, GL_FRAGMENT_SHADER, NULL }
    };

    GLuint program = glCreateProgram();

    // Compile and attach shaders
    for (int i = 0; i < 2; ++i) {
        Shader& s = shaders[i];
        s.source_ = ReadShaderSource(s.filename_);
        if (shaders[i].source_ == NULL) {
            std::cerr << "Failed to read " << s.filename_ << std::endl;
            exit(EXIT_FAILURE);
        }

        GLuint shader = glCreateShader(s.type_);
        glShaderSource(shader, 1, (const GLchar**)&s.source_, NULL);
        glCompileShader(shader);

        GLint compiled;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            std::cerr << s.filename_ << " failed to compile:" << std::endl;
            GLint logSize;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);
            char *logMsg = new char[logSize];
            glGetShaderInfoLog(shader, logSize, NULL, logMsg);
            std::cerr << logMsg << std::endl;
            delete []logMsg;
            exit(EXIT_FAILURE);
        }

        delete []s.source_;
        glAttachShader(program, shader);
    }

    // Link and error check
    glLinkProgram(program);

    GLint linked;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked) {
        std::cerr << "Shader program failed to link" << std::endl;
        GLint logSize;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logSize);
        char *logMsg = new char[logSize];
        glGetProgramInfoLog(program, logSize, NULL, logMsg);
        std::cerr << logMsg << std::endl;
        delete []logMsg;
        exit(EXIT_FAILURE);
    }

    // Use program object
    glUseProgram(program);
    return program;
}

}  // namespace angel
