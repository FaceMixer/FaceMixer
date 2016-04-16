//////////////////////////////////////////////////////////////////////////////
//
//  match.h
//
//  The header file for matching algorithm functions
//
//  Project         : Matching
//  Name            : Chong Guo
//  Student ID      : 301295753
//  SFU username    : armourg
//  Instructor      : Richard Zhang
//
//  Created by Armour on 3/26/2016
//  Copyright (c) 2016 Armour. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef MATCHING_MATCH_H
#define MATCHING_MATCH_H

#include <map>
#include <set>
#include <vector>
#include <queue>

#include "smf_parser.h"

using namespace std;

// Some extern variables
extern int import_semaphore;        // Import semaphore
extern int export_semaphore;        // Export semaphore
extern int mesh_imported;
extern GLUI_String file_path;       // Import/export file path
extern vector<smfparser::Vertex *> mesh_vertex;         // The vertex data that stored in memory
extern vector<smfparser::Face *> mesh_faces;            // The faces data that stored in memory
extern map<pair<int, int>, smfparser::W_edge *> mesh_edges;     // The edges data that stored in memory
extern map<smfparser::Vertex *, GLuint> vertex_index_map;
extern map<smfparser::Face *, GLuint> face_index_map;          // Mapping face to its index in mesh_faces
extern vector<GLfloat> render_vertex;       // The vertex data used for rendering
extern vector<GLuint> render_faces;         // The faces data used for rendering
extern vector<GLuint> render_edges;         // The edges data used for rendering
extern void UpdateMeshBufferData();

namespace match {

// Structure path
struct Path {
    int length;
    smfparser::Vertex *st;
    smfparser::Vertex *ed;
    vector<pair<int, int>> edges;
    Path(smfparser::Vertex *st, smfparser::Vertex *ed, int length): length(length), st(st), ed(ed) {         // Default constructor
        edges.clear();
    };
};

// Read the constrained vertex
void ReadConstrainedVertex(vector<int> &constrained_vertex, map<int, pair<float, float>> &constrained_vertex_position);

// Init the graph for dijkstra algorithm
void InitGraph(vector<smfparser::Vertex *> &match_vertex, map<pair<int, int>, smfparser::W_edge *> &match_edges);

// Find the shortest path among all pairs of constrained vertex
vector<Path *> FindShortestPath(vector<int> &constrained_vertex, map<pair<int, int>, smfparser::W_edge *> &match_edges);

// Check if this path is legal to choose
bool CheckLegal(Path *path, map<int, pair<float, float>> &constrained_vertex_position);

// Recompute the shortest path for specific st and ed vertex
Path *RecomputeShortestPath(int st, int ed, map<pair<int, int>, smfparser::W_edge *> &match_edges, vector<bool> &deleted_vertex);

// Output match result to file
void OutputPathMatchResult(vector<match::Path *> &TmVc, vector<int> &constrained_vertex,
                           map<int, pair<float, float>> &constrained_vertex_position);

}  // namespace match

#endif  // MATCHING_MATCH_H
