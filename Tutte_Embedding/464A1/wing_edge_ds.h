//
//  ds.h
//  smf_view
//
//  Created by Chen on 3/4/16.
//  Copyright © 2016 Qiao Chen. All rights reserved.
//

#ifndef ds_h
#define ds_h
#include "glm.hpp"
#include <OpenGL/OpenGL.h>
using namespace std;

/***** Winged-edge DS ****/

struct W_Vertex;
struct W_Face;
struct W_Edge
{
    W_Vertex *start=NULL, *end=NULL;
    W_Face *left=NULL, *right=NULL;
    W_Edge *left_prev=NULL, *left_next=NULL;
    W_Edge *right_prev=NULL,*right_next=NULL;
    //extra properties
    bool isboundary=false;
    
};
struct W_Vertex
{
    GLfloat x,y,z;
    W_Edge *edge;
    // extra properties
    int index;
    int internal_i;
    int boundary_i;
    bool isboundary=false;
    bool isfixed=false;
    bool haspushed=false;
    int valence;
};
struct W_Face
{
    W_Edge *edge;
    //extra properties
    glm::vec3 normal;
    int index;
};
struct adj_element
{
    bool adj=false;
    W_Edge *edge;
};
#endif /* ds_h */
