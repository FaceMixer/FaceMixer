//////////////////////////////////////////////////////////////////////////////
//
//  lib_gl.h
//
//  The header file for include OpenGL headers in different operating systems
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

#ifndef MATCHING_LIB_GL_H_
#define MATCHING_LIB_GL_H_

// Include OpenGL relevant headers
#ifdef __APPLE__
#  include <GL/glui.h>
#  include <GLUT/glut.h>
#  include <OpenGL/glu.h>
#  include <OpenGL/gl3.h>
#  include <glm/glm.hpp>
#  include <glm/gtc/matrix_transform.hpp>
#  include <glm/gtc/type_ptr.hpp>
#else   // End of Mac OSX header include
#  include <GL/glew.h>
#  include <GL/glui.h>
#  include <GL/freeglut.h>
#  include <GL/freeglut_ext.h>
#  include <glm/glm.hpp>
#  include <glm/gtc/matrix_transform.hpp>
#  include <glm/gtc/type_ptr.hpp>
#endif  // End of Linux header include

#endif  // MATCHING_LIB_GL_H_
