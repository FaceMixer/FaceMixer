//////////////////////////////////////////////////////////////////////////////
//
//  init_shader.h
//
//  The header file that offers an easy way to init vertex and fragment shaders
//
//  Project         : Matching
//  Name            : Chong Guo
//  Student ID      : 301295753
//  SFU username    : armourg
//  Instructor      : Richard Zhang
//
//  Created by Armour on 2/11/2016
//  Copyright (c) 2016 Armour. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef MATCHING_INIT_SHADER_H_
#define MATCHING_INIT_SHADER_H_

#include "lib_gl.h"

namespace angel {

// Helper function to read a shader source file and return its content
char *ReadShaderSource(const char *shader_file);

// Helper function to load vertex and fragment shader files
GLuint InitShader(const char* v_shader_file, const char* f_shader_file);

}  // namespace angle

#endif  // MATCHING_INIT_SHADER_H_
