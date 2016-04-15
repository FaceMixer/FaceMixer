//////////////////////////////////////////////////////////////////////////////
//
//  main.cc
//
//  The source file for smf-view project, including most of the OpenGL functions
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

#include <stdlib.h>
#include <iostream>
#include <vector>
#include <map>

#include "lib_gl.h"
#include "lib_consts.h"
#include "init_shader.h"
#include "smf_parser.h"
#include "match.h"

using namespace std;

int main_win;       // The id of main window
int import_semaphore = libconsts::kImportLockOff;       // Import semaphore
int export_semaphore = libconsts::kExportLockOff;       // Export semaphore
int mesh_imported = libconsts::kMeshImportedFalse;      // The mesh import flag (for display)
int mesh_type = libconsts::kMeshTypeWireFrame;

// Variables in GLUI
GLUI_String file_path = "face.smf";      // The string of file path
GLUI *gluiRight;            // The GLUI on right
GLUI *gluiBot;              // The GLUI on bottom
int colorful = 0;           // The flag for colorful display
int matched_display = 0;    // The flag for matched mesh display

// The mesh data that stored in winged_edge structure
vector<smfparser::Vertex *> mesh_vertex;    // The vertex mesh data
vector<smfparser::Face *> mesh_faces;       // The faces mesh data
map<pair<int, int>, smfparser::W_edge *> mesh_edges;    // The edges mesh data
map<smfparser::Vertex *, GLuint> vertex_index_map;      // Mapping vertex to its index in mesh_vertex
map<smfparser::Face *, GLuint> face_index_map;          // Mapping face to its index in mesh_faces

// The data used for rendering
vector<GLfloat> render_vertex;    // The vertex data used for rendering
vector<GLuint> render_faces;      // The faces data used for rendering
vector<GLuint> render_edges;      // The edges data used for rendering
vector<GLuint> render_matched_edges;    // The matched edges data used for rendering
vector<GLuint> render_constrained_vertices;    // The constrained vertices data used for rendering

// The data used for matching algorithm
vector<int> constrained_vertex;                 // Constrained vertices for matching
vector<smfparser::Vertex *> match_vertex;       // The vertex data used for matching algorithm
vector<bool> deleted_vertex;                    // The flag that indicate if a vertex has been deleted
map<pair<int, int>, smfparser::W_edge *> match_edges;    // The edges data used for matching algorithm
map<int, pair<float, float>> constrained_vertex_position;   // The constrained vertex position
vector<match::Path *> shortest_path;            // The shortest path for each pair of constrained vertices in mesh
vector<match::Path *> TmVc;
vector<pair<int, int>> TPc;

// MVP matrix
glm::mat4 MVP;
glm::mat4 MV;

// Location of vertex attributes in the shader program
GLuint v_position;
GLuint v_mvp;
GLuint v_mv;
GLuint v_color;
GLuint is_wireframe;
GLuint is_colorful;
GLuint is_smooth;

// VAO and VBO
GLuint vao_IDs[4];      // VAO for each object: one for solid, one for wireframe
GLuint vbo_IDs[4];      // Vertex Buffer Object for each VAO (specifying vertex positions and colors)
GLuint ebo_IDs[4];      // Element Array Buffer Object for each VAO (one for solid, two for wireframe, one for dot)

// The rotation and position that changed by GLUI controller
GLfloat lights_rotation[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
GLfloat object_rotation[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
GLfloat object_position[3]  = {0,0,0};

//
// Function: Compare
// ---------------------------
//
//   The compare function for sorting the shortest path
//
//   Parameters:
//       void
//
//   Returns:
//       void
//

bool Compare(const match::Path *a, const match::Path *b) {
    return a->length > b->length;
}

//
// Function: StartMatching
// ---------------------------
//
//   Start matching
//
//   Parameters:
//       void
//
//   Returns:
//       void
//

void StartMatching(void) {
    if (!mesh_imported) return;

    match::ReadConstrainedVertex(constrained_vertex, constrained_vertex_position);       // Read in constrained vertices

    match_edges = mesh_edges;
    match_vertex = mesh_vertex;
    deleted_vertex.clear();
    deleted_vertex.assign(match_vertex.size(), false);

    cout << "Start init graph..." << endl;
    match::InitGraph(match_vertex, match_edges);            // Init graph for dijkstra algorithm
    cout << "Graph init finished!" << endl;

    cout << "Start calculate shortest paths..." << endl;
    shortest_path = match::FindShortestPath(constrained_vertex, match_edges);       // Get all pairs of shortest path
    sort(shortest_path.begin(), shortest_path.end(), Compare);                      // Sort shortest path array
    cout << "Shortest paths calculation finished!" << endl;

    while (!shortest_path.empty()) {                        // Do until no more shortest path
        cout << "Remaining shortest path: " << shortest_path.size() << endl;
        match::Path *path = shortest_path.back();
        shortest_path.pop_back();
        if (match::CheckLegal(path, constrained_vertex_position)) {     // If this edge is legal to add to the final set
            TmVc.push_back(path);
            TPc.push_back(make_pair(vertex_index_map[path->st] + 1, vertex_index_map[path->ed] + 1));
            for (int i = 0; i < path->edges.size(); i++) {              // Delete all interior vertices of the chosen path
                deleted_vertex[path->edges[i].first - 1] = true;
                deleted_vertex[path->edges[i].second - 1] = true;
            }
            for (int i = 0; i < shortest_path.size(); i++ ) {           // Check every remain path to see if it need update
                int flag = false;
                for (int j = 0; j < shortest_path[i]->edges.size(); j++) {
                    if (deleted_vertex[shortest_path[i]->edges[j].first - 1] && (j != 0) ||
                        deleted_vertex[shortest_path[i]->edges[j].second - 1] && (j != shortest_path[i]->edges.size() - 1)) {
                        flag = true;
                        break;
                    }
                }
                if (flag) {     // If contain deleted vertices, recompute the shortest path for this pair of vertices
                    shortest_path[i] = match::RecomputeShortestPath(vertex_index_map[shortest_path[i]->st],
                                                                    vertex_index_map[shortest_path[i]->ed],
                                                                    match_edges, deleted_vertex);
                }
            }
            sort(shortest_path.begin(), shortest_path.end(), Compare);      // Sort shortest path array after update
        }
    }

    render_matched_edges.clear();
    for (auto p: TmVc) {        // Put all matched edges to render array
        for (int i = 0; i < p->edges.size(); i++) {
            render_matched_edges.push_back(p->edges[i].first - 1);
            render_matched_edges.push_back(p->edges[i].second - 1);
        }
    }
    UpdateMeshBufferData();

    cout << "Start to output all patches with boundary path to file..." << endl;
    match::OutputPathMatchResult(TmVc, constrained_vertex, constrained_vertex_position);        // Output match result to file
    cout << "Output finished!" << endl;
}

//
// Function: UpdateMVP
// ---------------------------
//
//   Update MVP matrix
//
//   Parameters:
//       void
//
//   Returns:
//       void
//

void UpdateMVP() {
    // Projection matrix : 45 degree Field of View, 4:3 ratio, display range 0.05 unit - 100 units
    glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.05f, 100.0f);

    // Camera matrix
    glm::mat4 View = glm::lookAt(
            glm::vec3(0.0 - object_position[0], 0.0 - object_position[1], 5.0 - object_position[2]), // Camera position
            glm::vec3(-object_position[0], -object_position[1], -object_position[2]), // Looks at the relative origin
            glm::vec3(0, 1, 0)  // Head is up (set to 0, -1, 0 to look upside-down)
    );

    // Model matrix : an identity matrix (model will be at the origin)
    glm::mat4 rotation;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            rotation[i][j] = object_rotation[i * 4 + j];
    glm::mat4 Model = rotation * glm::mat4(1.0f);  // Changes by GLUI rotation controller

    // Our ModelViewProjection : multiplication of our 3 matrices
    MVP = Projection * View * Model;
    MV = View * Model;
}

//
// Function: UpdateMeshBufferData
// ---------------------------
//
//   Update Mesh Buffer Data
//
//   Parameters:
//       void
//
//   Returns:
//       void
//

void UpdateMeshBufferData() {
    // Update first VAO
    glBindVertexArray(vao_IDs[0]);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_IDs[0]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, render_vertex.size() * sizeof(GLfloat), &render_vertex.front());

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_IDs[0]);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, render_faces.size() * sizeof(GLuint), &render_faces.front());

    // Update second VAO
    glBindVertexArray(vao_IDs[1]);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_IDs[1]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, render_vertex.size() * sizeof(GLfloat), &render_vertex.front());

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_IDs[1]);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, render_edges.size() * sizeof(GLuint), &render_edges.front());

    // Update third VAO
    glBindVertexArray(vao_IDs[2]);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_IDs[2]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, render_vertex.size() * sizeof(GLfloat), &render_vertex.front());

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_IDs[2]);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, render_matched_edges.size() * sizeof(GLuint), &render_matched_edges.front());

    // Update third VAO
    glBindVertexArray(vao_IDs[3]);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_IDs[3]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, render_vertex.size() * sizeof(GLfloat), &render_vertex.front());

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_IDs[3]);
    render_constrained_vertices.clear();
    for (auto i : constrained_vertex) {
        render_constrained_vertices.push_back(i - 1);
    }
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, render_constrained_vertices.size() * sizeof(GLuint), &render_constrained_vertices.front());
}

//
// Function: Init
// ---------------------------
//
//   Init OpenGL lighting, materials, shaders and VAO, VBO
//
//   Parameters:
//       void
//
//   Returns:
//       void
//

void Init() {
    // Enable Z-buffering
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // Load shaders and use the shader program
#ifdef __APPLE__
    GLuint program = angel::InitShader("vshader_mac.glsl", "fshader_mac.glsl");
#else
    GLuint program = angel::InitShader("vshader_unix.glsl", "fshader_unix.glsl");
#endif
    glUseProgram(program);

    // Get the attribute and uniform variable location in shader
    v_position = (GLuint)glGetAttribLocation(program, "v_position");
    v_color = (GLuint)glGetAttribLocation(program, "v_color");
    v_mvp = (GLuint)glGetUniformLocation(program, "v_mvp");
    v_mv = (GLuint)glGetUniformLocation(program, "v_mv");
    is_wireframe = (GLuint)glGetUniformLocation(program, "is_wireframe");
    is_colorful = (GLuint)glGetUniformLocation(program, "is_colorful");
    is_smooth = (GLuint)glGetUniformLocation(program, "is_smooth");

    // Create Vertex Array Objects, store the names in array vao_IDs
    glGenVertexArrays(4, vao_IDs);

    // Bind for first VAO
    glBindVertexArray(vao_IDs[0]);
    glGenBuffers(1, &vbo_IDs[0]);
    glGenBuffers(1, &ebo_IDs[0]);

    // Create Array Buffer Object, store the names in array vbo_IDs
    glBindBuffer(GL_ARRAY_BUFFER, vbo_IDs[0]);
    glBufferData(GL_ARRAY_BUFFER, libconsts::kMaxVertexNum * libconsts::kElementVecLenTotal * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(v_position);
    glVertexAttribPointer(v_position, libconsts::kElementVecLenPosition, GL_FLOAT, GL_FALSE, libconsts::kElementVecLenTotal * sizeof(GLfloat), 0);

    // Create Element Array Buffer Object, store the names in array ebo_IDs
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_IDs[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, libconsts::kMaxFaceNum * 3 * sizeof(GLuint), NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(v_color);
    glVertexAttribPointer(v_color, libconsts::kElementVecLenColor, GL_FLOAT, GL_FALSE, libconsts::kElementVecLenTotal * sizeof(GLfloat),
                          (void *)(libconsts::kElementVecLenPosition * sizeof(GLfloat)));

    // Bind for second VAO
    glBindVertexArray(vao_IDs[1]);
    glGenBuffers(1, &vbo_IDs[1]);
    glGenBuffers(1, &ebo_IDs[1]);

    // Create Array Buffer Object, store the names in array vbo_IDs
    glBindBuffer(GL_ARRAY_BUFFER, vbo_IDs[1]);
    glBufferData(GL_ARRAY_BUFFER, libconsts::kMaxVertexNum * libconsts::kElementVecLenTotal * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(v_position);
    glVertexAttribPointer(v_position, libconsts::kElementVecLenPosition, GL_FLOAT, GL_FALSE, libconsts::kElementVecLenTotal * sizeof(GLfloat), 0);

    // Create Element Array Buffer Object, store the names in array ebo_IDs
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_IDs[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, libconsts::kMaxEdgeNum * 2 * sizeof(GLuint), NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(v_color);
    glVertexAttribPointer(v_color, libconsts::kElementVecLenColor, GL_FLOAT, GL_FALSE, libconsts::kElementVecLenTotal * sizeof(GLfloat),
                          (void *)(libconsts::kElementVecLenPosition * sizeof(GLfloat)));

    // Bind for third VAO
    glBindVertexArray(vao_IDs[2]);
    glGenBuffers(1, &vbo_IDs[2]);
    glGenBuffers(1, &ebo_IDs[2]);

    // Create Array Buffer Object, store the names in array vbo_IDs
    glBindBuffer(GL_ARRAY_BUFFER, vbo_IDs[2]);
    glBufferData(GL_ARRAY_BUFFER, libconsts::kMaxVertexNum * libconsts::kElementVecLenTotal * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(v_position);
    glVertexAttribPointer(v_position, libconsts::kElementVecLenPosition, GL_FLOAT, GL_FALSE, libconsts::kElementVecLenTotal * sizeof(GLfloat), 0);

    // Create Element Array Buffer Object, store the names in array ebo_IDs
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_IDs[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, libconsts::kMaxEdgeNum * 2 * sizeof(GLuint), NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(v_color);
    glVertexAttribPointer(v_color, libconsts::kElementVecLenColor, GL_FLOAT, GL_FALSE, libconsts::kElementVecLenTotal * sizeof(GLfloat),
                          (void *)(libconsts::kElementVecLenPosition * sizeof(GLfloat)));

    // Bind for fourth VAO
    glBindVertexArray(vao_IDs[3]);
    glGenBuffers(1, &vbo_IDs[3]);
    glGenBuffers(1, &ebo_IDs[3]);

    // Create Array Buffer Object, store the names in array vbo_IDs
    glBindBuffer(GL_ARRAY_BUFFER, vbo_IDs[3]);
    glBufferData(GL_ARRAY_BUFFER, libconsts::kMaxVertexNum * libconsts::kElementVecLenTotal * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(v_position);
    glVertexAttribPointer(v_position, libconsts::kElementVecLenPosition, GL_FLOAT, GL_FALSE, libconsts::kElementVecLenTotal * sizeof(GLfloat), 0);

    // Create Element Array Buffer Object, store the names in array ebo_IDs
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_IDs[3]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, libconsts::kMaxVertexNum * sizeof(GLuint), NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(v_color);
    glVertexAttribPointer(v_color, libconsts::kElementVecLenColor, GL_FLOAT, GL_FALSE, libconsts::kElementVecLenTotal * sizeof(GLfloat),
                          (void *)(libconsts::kElementVecLenPosition * sizeof(GLfloat)));
}

//
// Function: DisplayFunc
// ---------------------------
//
//   The display callback function
//
//   Parameters:
//       void
//
//   Returns:
//       void
//

void DisplayFunc(void) {
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    UpdateMVP();    // Update MVP matrix

    gluiBot->sync_live();       // Sync live variables in GLUI

    if (mesh_imported == libconsts::kMeshImportedTrue) {
        glUniform1i(is_colorful, !colorful);
        glUniformMatrix4fv(v_mvp, 1, GL_FALSE, glm::value_ptr(MVP));        // Send MVP matrix to uniform v_mvp in shader
        glUniformMatrix4fv(v_mv, 1, GL_FALSE, glm::value_ptr(MV));          // Send MV matrix to uniform v_mv in shader
        if (mesh_type == libconsts::kMeshTypeSmoothShaded) {                // Smooth shaded
            glUniform1i(is_smooth, 1);
            glUniform1i(is_wireframe, 0);
            glBindVertexArray(vao_IDs[0]);
            glDrawElements(GL_TRIANGLES, render_faces.size(), GL_UNSIGNED_INT, 0);
        } else if (mesh_type == libconsts::kMeshTypeFlatShaded) {           // Flat Shaded
            glUniform1i(is_smooth, 0);
            glUniform1i(is_wireframe, 0);
            glBindVertexArray(vao_IDs[0]);
            glDrawElements(GL_TRIANGLES, render_faces.size(), GL_UNSIGNED_INT, 0);
        } else if (mesh_type == libconsts::kMeshTypeWireFrame) {            // Wire frame
            glUniform1i(is_smooth, 1);
            glUniform1i(is_wireframe, 1);
            if (!matched_display) {
                glBindVertexArray(vao_IDs[1]);
                glDrawElements(GL_LINES, render_edges.size(), GL_UNSIGNED_INT, 0);
            } else {
                glBindVertexArray(vao_IDs[2]);
                glDrawElements(GL_LINES, render_matched_edges.size(), GL_UNSIGNED_INT, 0);
            }
            glBindVertexArray(vao_IDs[3]);
            glDrawElements(GL_POINTS, render_constrained_vertices.size(), GL_UNSIGNED_INT, 0);
        } else {                                                            // Solid + wire frame
            glUniform1i(is_smooth, 1);

            // First draw solid object with polygon offset
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(1.0f, 1.0f);
            glUniform1i(is_wireframe, 0);
            glBindVertexArray(vao_IDs[0]);
            glDrawElements(GL_TRIANGLES, render_faces.size(), GL_UNSIGNED_INT, 0);
            glDisable(GL_POLYGON_OFFSET_FILL);

            // Then draw wireframe without polygon offset
            glUniform1i(is_wireframe, 1);
            if (!matched_display) {
                glBindVertexArray(vao_IDs[1]);
                glDrawElements(GL_LINES, render_edges.size(), GL_UNSIGNED_INT, 0);
            } else {
                glBindVertexArray(vao_IDs[2]);
                glDrawElements(GL_LINES, render_matched_edges.size(), GL_UNSIGNED_INT, 0);
            }
            glBindVertexArray(vao_IDs[3]);
            glDrawElements(GL_POINTS, render_constrained_vertices.size(), GL_UNSIGNED_INT, 0);
        }
    }

    glutSwapBuffers();
}

//
// Function: ReshapeFunc
// ---------------------------
//
//   The reshape callback function
//
//   Parameters:
//       not used
//
//   Returns:
//       void
//

void ReshapeFunc(int, int) {
    GLUI_Master.auto_set_viewport();        // GLUI auto set the view port, cheers!
    glutPostRedisplay();
}

//
// Function: KeyboardFunc
// ---------------------------
//
//   The keyboard callback function
//
//   Parameters:
//       key: the key that pressed by user
//
//   Returns:
//       void
//

void KeyboardFunc(unsigned char key, int, int) {
    switch (key) {
        case 033:       // Both escape key and 'q' cause the game to exit
            exit(EXIT_SUCCESS);
        case 'q':       // Both escape key and 'q' cause the game to exit
            exit(EXIT_SUCCESS);
        case 'r':       // Reset all the parameters in GLUI
            mesh_type = libconsts::kMeshTypeFlatShaded;
            for (int i = 0; i < 16; i++)        // Reset object rotation
                object_rotation[i] = 0.0f + (i % 5 == 0);
            for (int i = 0; i < 3; i++)         // Reset object position
                object_position[i] = 0.0f;
            for (int i = 0; i < 16; i++)        // Reset light rotation
                lights_rotation[i] = 0.0f + (i % 5 == 0);
            gluiRight->sync_live();             // Sync live variables in GLUI
            gluiBot->sync_live();
            break;
        case '1':       // Change to Flat Shaded mesh type
            mesh_type = libconsts::kMeshTypeFlatShaded;
            gluiRight->sync_live();
            break;
        case '2':       // Change to Smooth Shaded mesh type
            mesh_type = libconsts::kMeshTypeSmoothShaded;
            gluiRight->sync_live();
            break;
        case '3':       // Change to Wire Frame mesh type
            mesh_type = libconsts::kMeshTypeWireFrame;
            gluiRight->sync_live();
            break;
        case '4':       // Change to Shaded Edges mesh type
            mesh_type = libconsts::kMeshTypeShadedEdges;
            gluiRight->sync_live();
            break;
        case 'i':       // Import the mesh file
            smfparser::ImportMeshFile();
            break;
        case 'e':       // Export the mesh file
            smfparser::ExportMeshFile();
            break;
    }
    glutPostRedisplay();
}

//
// Function: IdleFunc
// ---------------------------
//
//   The idle callback function
//
//   Parameters:
//       void
//
//   Returns:
//       void
//

void IdleFunc(void) {
    glutSetWindow(main_win);
    glutPostRedisplay();
}

//
// Function: InitGLUI
// ---------------------------
//
//   Init GLUI window
//
//   Parameters:
//       void
//
//   Returns:
//       void
//

void InitGLUI(void) {
    // Temp GLUI to fix MAC display bug
#ifdef __APPLE__
    GLUI *tmp = GLUI_Master.create_glui("Temp", 0, 0, 0);
    tmp->hide();
#endif

    // Setup right subwindow GUI
    gluiRight = GLUI_Master.create_glui_subwindow(main_win, GLUI_SUBWINDOW_RIGHT);

    // Add source panel
    GLUI_Panel *source_panel = gluiRight->add_panel("Source");
    gluiRight->add_column_to_panel(source_panel, false);
    gluiRight->add_column_to_panel(source_panel, false);
    gluiRight->add_edittext_to_panel(source_panel, "File Path", file_path);
    gluiRight->add_button_to_panel(source_panel, "Open", 0, (GLUI_Update_CB)smfparser::ImportMeshFile);
    gluiRight->add_button_to_panel(source_panel, "Save", 0, (GLUI_Update_CB)smfparser::ExportMeshFile);
    gluiRight->add_column_to_panel(source_panel, false);
    gluiRight->add_column_to_panel(source_panel, false);

    // Add properties panel
    GLUI_Panel *properties_panel = gluiRight->add_panel("Properties");
    GLUI_Listbox *mesh_listbox = gluiRight->add_listbox_to_panel(properties_panel, "Mesh Type ", &mesh_type);
    mesh_listbox->add_item(libconsts::kMeshTypeFlatShaded, "flat shaded");
    mesh_listbox->add_item(libconsts::kMeshTypeSmoothShaded, "smooth shaded");
    mesh_listbox->add_item(libconsts::kMeshTypeWireFrame, "wireframe");
    mesh_listbox->add_item(libconsts::kMeshTypeShadedEdges, "shaded edges");

    // Add color checkbox
    GLUI_Panel *color_panel = gluiRight->add_panel("Color");
    gluiRight->add_column_to_panel(color_panel, false);
    GLUI_RadioGroup *color_radio_group = gluiRight->add_radiogroup_to_panel(color_panel, &colorful);
    gluiRight->add_radiobutton_to_group(color_radio_group, "Colorful");
    gluiRight->add_radiobutton_to_group(color_radio_group, "White");
    gluiRight->add_column_to_panel(color_panel, false);

    // Add display checkbox
    GLUI_Panel *display_panel = gluiRight->add_panel("Display");
    gluiRight->add_column_to_panel(display_panel, false);
    GLUI_RadioGroup *display_radio_group = gluiRight->add_radiogroup_to_panel(display_panel, &matched_display);
    gluiRight->add_radiobutton_to_group(display_radio_group, "Original");
    gluiRight->add_radiobutton_to_group(display_radio_group, "Matched");
    gluiRight->add_column_to_panel(display_panel, false);

    // Matching button
    gluiRight->add_button("Matching", 0, (GLUI_Update_CB)StartMatching);

    // Add quit button
    gluiRight->add_button("Quit", 0, (GLUI_Update_CB)exit);

    // Set main gfx windows
    gluiRight->set_main_gfx_window(main_win);

    // Setup bottom subwindow GUI
    gluiBot = GLUI_Master.create_glui_subwindow(main_win, GLUI_SUBWINDOW_BOTTOM);

    // Add rotation control
    gluiBot->add_rotation("Rotation", object_rotation);
    gluiBot->add_column(false);

    // Add translation control
    GLUI_Translation *txy = gluiBot->add_translation("Object XY", GLUI_TRANSLATION_XY, object_position);
    gluiBot->add_column(false);
    GLUI_Translation *tx = gluiBot->add_translation("Object X", GLUI_TRANSLATION_X, &object_position[0]);
    gluiBot->add_column(false);
    GLUI_Translation *ty = gluiBot->add_translation("Object Y", GLUI_TRANSLATION_Y, &object_position[1]);
    gluiBot->add_column(false);
    GLUI_Translation *tz = gluiBot->add_translation("Object Z", GLUI_TRANSLATION_Z, &object_position[2]);

    // Change translation scale factors
    txy->scale_factor = 0.01f;
    tx->scale_factor = 0.01f;
    ty->scale_factor = 0.01f;
    tz->scale_factor = 0.01f;

    // Set main gfx windows
    gluiBot->set_main_gfx_window(main_win);
}

//
// Function: Main
// ---------------------------
//
//   The main function
//
//   Parameters:
//       argc: the number of parameters in main function
//       argv[]: the array of parameters in main function
//
//   Returns:
//       void
//

int main(int argc, char *argv[]) {
    // Initialize GLUT and create window
    glutInit(&argc, argv);
#ifdef __APPLE__
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#else
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#endif
    glutInitWindowPosition(libconsts::kWindowPositionX, libconsts::kWindowPositionY);
    glutInitWindowSize(libconsts::kWindowSizeWidth, libconsts::kWindowSizeHeight);

    // Setup callback function
    main_win = glutCreateWindow("Window");
    GLUI_Master.set_glutReshapeFunc(ReshapeFunc);
    GLUI_Master.set_glutKeyboardFunc(KeyboardFunc);
    GLUI_Master.set_glutIdleFunc(IdleFunc);
    glutDisplayFunc(DisplayFunc);

#ifndef __APPLE__
    glewInit();
#endif

    // Init
    Init();
    InitGLUI();

    // Main loop
    glutMainLoop();
}
