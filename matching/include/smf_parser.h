//////////////////////////////////////////////////////////////////////////////
//
//  smf_parser.h
//
//  The header file for function that import/export mesh data file
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

#ifndef SMFVIEW_SMF_PARSER_H_
#define SMFVIEW_SMF_PARSER_H_

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>

#include "lib_gl.h"
#include "lib_consts.h"
#include "winged_edge.h"

using namespace std;

// Some extern variables
extern int import_semaphore;        // Import semaphore
extern int export_semaphore;        // Export semaphore
extern int mesh_imported;
extern GLUI_String file_path;       // Import/export file path
extern vector<smfparser::Vertex *> mesh_vertex;     // The vertex data that stored in memory
extern vector<smfparser::Face *> mesh_faces;        // The faces data that stored in memory
extern map<pair<int, int>, smfparser::W_edge *> mesh_edges;     // The edges data that stored in memory
extern map<smfparser::Vertex *, GLuint> vertex_index_map;
extern vector<GLfloat> data_vertex;     // The vertex data used for rendering
extern vector<GLuint> data_faces;       // The faces data used for rendering
extern vector<GLuint> data_edges;       // The edges data used for rendering
extern void UpdateMeshBufferData();

namespace smfparser {

// Trim the leading space of a string
void TrimLeadingSpace(string &s);

// Trim the tailing space of a string
void TrimTailingSpace(string &s);

// Import mesh data from file to memory
void ImportMeshFile();

// Export mesh data from memory to file
void ExportMeshFile();

// Init mesh data for rendering
void InitRenderMeshData();

} // namespace smfparser

#endif // SMFVIEW_SMF_PARSER_H_
