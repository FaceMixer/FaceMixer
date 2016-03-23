//////////////////////////////////////////////////////////////////////////////
//
//  winged_edge.h
//
//  The header file for smf-view winged-edge structure declaration
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

#ifndef SMFVIEW_WINGED_EDGE_H_
#define SMFVIEW_WINGED_EDGE_H_

#include "lib_gl.h"

namespace smfparser {

struct Vertex;
struct W_edge;
struct Face;

// Structure W_edge
struct W_edge {
    Vertex *start, *end;
    Face *left, *right;
    W_edge *left_prev, *right_prev;
    W_edge *left_next, *right_next;
    float cost;
    W_edge(Vertex *st, Vertex *ed): start(st), end(ed), cost(0) {        // Default constructor
        left = nullptr;
        right = nullptr;
        left_prev = nullptr;
        left_next = nullptr;
        right_prev = nullptr;
        right_next = nullptr;
    }
};

// Structure Vertex
struct Vertex {
    W_edge *edge;
    glm::mat4x4 *Q;
    float x, y, z;
    int render_flag;
    Vertex(float x, float y, float z): edge(nullptr), Q(nullptr), x(x), y(y), z(z), render_flag(0) {};         // Default constructor
};

// Structure Face
struct Face {
    W_edge *edge;
    glm::mat4x4 *K;
    int render_flag;
    Face(): edge(nullptr), K(nullptr), render_flag(0) {};      // Default constructor
};

} // namespace smfparser

#endif // SMFVIEW_WINGED_EDGE_H_
