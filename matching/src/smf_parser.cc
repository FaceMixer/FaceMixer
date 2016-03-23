//////////////////////////////////////////////////////////////////////////////
//
//  smf_parser.cc
//
//  The source file for function that import/export mesh data file
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

#include "smf_parser.h"

namespace smfparser {

//
// Function: Split
// ---------------------------
//
//   A template function for string split
//
//   Parameters:
//       T: the class type
//       s: the string that needs to split
//       delim: the delaminator that used to split s
//       ret: the returned vector made by splited elements
//
//   Returns:
//       void
//

template <class T>
void split(string &s, char delim, vector<T> &ret) {
    stringstream ss(s);
    string str;
    while (getline(ss, str, delim)) {
        if (str.compare("") != 0)
            ret.push_back((T)stof(str));
    }
}

//
// Function: TrimLeadingSpace
// ---------------------------
//
//   Trim the leading space of a string
//
//   Parameters:
//       s: the string that needs to trim
//
//   Returns:
//       void
//

void TrimLeadingSpace(string &s) {
    size_t pos = s.find_first_not_of(' ');
    if (pos != string::npos && pos != 0) {
        s.erase(0, pos - 1);
    }
}

//
// Function: TrimTailingSpace
// ---------------------------
//
//   Trim the tailing space of a string
//
//   Parameters:
//       s: the string that needs to trim
//
//   Returns:
//       void
//

void TrimTailingSpace(string &s) {
    size_t pos = s.find_last_not_of(" \r\n");
    if (pos != string::npos) {
        s.erase(pos + 1);
    }
}

//
// Function: ImportMeshFile
// ---------------------------
//
//   Import mesh data from file to memory
//
//   Parameters:
//       void
//
//   Returns:
//       void
//

void ImportMeshFile() {
    mesh_imported = libconsts::kMeshImportedFalse;      // Init mesh import flag (for display)

    if (import_semaphore == libconsts::kImportLockOff) {    // Check import semaphore (for mutual exclusion)
        import_semaphore = libconsts::kImportLockOn;

        string line;
        ifstream fin("mesh/" + file_path, ifstream::in);

        if (!fin.is_open()) {       // Check if input file exist
            cout << "Mesh file doesn't exist!" << endl;
            import_semaphore = libconsts::kImportLockOff;
            return;
        }

        int flag = libconsts::kFlagVertex;      // Init flag and clear mesh data
        mesh_vertex.clear();
        mesh_edges.clear();
        mesh_faces.clear();

        while (getline(fin, line)) {        // While not end of file
            if (line.find_first_not_of(' ') == string::npos) continue;    // If is blank line
            TrimLeadingSpace(line);                 // Trim leading space
            if (line[0] == '#') continue;           // If is comment
            if (line[0] == 'f') flag = libconsts::kFlagFace;
            line.erase(0, 2);
            TrimTailingSpace(line);                 // Trim tailing space
            if (flag == libconsts::kFlagVertex) {       // If this line contains vertex data
                vector<float> pos;
                split(line, ' ', pos);
                if (pos.size() >= 3) {
                    smfparser::Vertex *new_vertex = new smfparser::Vertex(pos[0], pos[1], pos[2]);      // Add new vertex
                    mesh_vertex.push_back(new_vertex);
                    vertex_index_map[new_vertex] = (GLuint)(mesh_vertex.size() - 1);
                } else {                                // If size is less than 3, then this is not a valid smf file
                    cout << "Fatal! Smf file import error!" << endl;
                    exit(EXIT_FAILURE);
                }
            } else {
                vector<int> index;
                split(line, ' ', index);
                int size = (int)index.size();
                if (size >= 3) {
                    smfparser::Face *new_face = new smfparser::Face();          // Add new face
                    mesh_faces.push_back(new_face);
                    for (int i = 0; i < size; i++) {    // Add all edges around this face
                        smfparser::Vertex *st = mesh_vertex[index[i] - 1];
                        smfparser::Vertex *ed = mesh_vertex[index[(i + 1) % size] - 1];
                        smfparser::W_edge *new_edge = new smfparser::W_edge(st, ed);        // Add new edge from st to ed
                        pair<int, int> p(index[i], index[(i + 1) % size]);
                        mesh_edges[p] = new_edge;
                        new_face->edge = new_edge;      // Choose any adjacent edge
                        new_edge->left = new_face;      // Edge left face
                        st->edge = new_edge;            // Choose any adjacent edge
                    }
                    for (int i = 0; i < size; i++) {    // Update all edges data around this face
                        pair<int, int> p(index[i], index[(i + 1) % size]);
                        pair<int, int> p_prev(index[(i - 1 + size) % size], index[i]);
                        pair<int, int> p_next(index[(i + 1) % size], index[(i + 2) % size]);
                        pair<int, int> p_reverse(index[(i + 1) % size], index[i]);
                        smfparser::W_edge *edge = mesh_edges.find(p)->second;               // This edge
                        smfparser::W_edge *edge_prev = mesh_edges.find(p_prev)->second;     // The previous edge
                        smfparser::W_edge *edge_next = mesh_edges.find(p_next)->second;     // The next edge
                        edge->left_prev = edge_prev;
                        edge->left_next = edge_next;
                        if (mesh_edges.find(p_reverse) != mesh_edges.end()) {       // Update edge information according to its reverse
                            smfparser::W_edge *edge2 = mesh_edges.find(p_reverse)->second;
                            edge->right = edge2->left;
                            edge->right_prev = edge2->left_prev;
                            edge->right_next = edge2->left_next;
                            edge2->right = edge->left;
                            edge2->right_prev = edge->left_prev;
                            edge2->right_next = edge->left_next;
                        }
                    }
                } else {        // If size is less than 3, then this is not a valid smf file
                    cout << "Fatal! Smf file import error!" << endl;
                    exit(EXIT_FAILURE);
                }
            }
        }

        fin.close();

        InitRenderMeshData();       // Init the mesh data for rendering
        UpdateMeshBufferData();     // Update mesh data in the buffer

        import_semaphore = libconsts::kImportLockOff;   // Update import semaphore (for mutual exclusion)
        mesh_imported = libconsts::kMeshImportedTrue;   // Update mesh import flag (for display)

        cout << "Import Finished" << endl;
    }
}

//
// Function: ImportMeshFile
// ---------------------------
//
//   Export mesh data from memory to file
//
//   Parameters:
//       void
//
//   Returns:
//       void
//

void ExportMeshFile() {
    if (export_semaphore == libconsts::kExportLockOff) {    // Check export semaphore (for mutual exclusion)
        export_semaphore = libconsts::kExportLockOn;

        if (mesh_vertex.empty()) {      // Check if mesh data is empty
            cout << "No mesh data exist now!" << endl;
            export_semaphore = libconsts::kExportLockOff;
            return;
        }

        ofstream fout("mesh/" + file_path, ofstream::out);

        if (!fout.is_open()) {      // Check if output file created
            cout << "Output file create failed!" << endl;
            export_semaphore = libconsts::kExportLockOff;
            return;
        }

        int extra_vertex = 0;
        int extra_face = 0;
        vector<int> extra_prefix;

        for (auto vertex : mesh_vertex) {
            if (vertex->render_flag == 1) extra_vertex++;
            extra_prefix.push_back(extra_vertex);
        }

        for (auto face : mesh_faces)
            if (face->render_flag == 1) extra_face++;

        // The vertex size and faces size
        fout << "# " << mesh_vertex.size() - extra_vertex << " " << mesh_faces.size() - extra_face << endl;

        // information
        fout << "# Created by Armour on 2016" << endl;
        fout << "# Copyright (c) 2016 Armour. All rights reserved." << endl;
        fout << endl;

        for (auto vertex : mesh_vertex) {
            if (vertex->render_flag == 1) continue;
            fout << "v " << vertex->x << " " << vertex->y << " " << vertex->z << endl;      // The vertex data
        }

        for (auto face : mesh_faces) {
            if (face->render_flag == 1) continue;
            smfparser::W_edge *e0 = face->edge;
            smfparser::W_edge *edge = e0;
            fout << "f";
            do {
                fout << " " << vertex_index_map[edge->start] + 1 - extra_prefix[vertex_index_map[edge->start]];      // The faces data
                if (edge->left == face)
                    edge = edge->left_next;
                else
                    edge = edge->right_next;
            } while (edge != e0);
            fout << endl;
        }

        fout.close();

        export_semaphore = libconsts::kExportLockOff;   // Update export semaphore (for mutual exclusion)
        cout << "Export Finished" << endl;
    }
}

//
// Function: InitRenderMeshData
// ---------------------------
//
//   Init mesh data for rendering
//
//   Parameters:
//       void
//
//   Returns:
//       void
//

void InitRenderMeshData() {
    data_vertex.clear();    // Clear mesh data for rendering
    data_faces.clear();
    data_edges.clear();

    for (auto vertex : mesh_vertex) {       // Update vertex data for rendering
        data_vertex.push_back(vertex->x);
        data_vertex.push_back(vertex->y);
        data_vertex.push_back(vertex->z);
        data_vertex.push_back((float)(rand() % 100) / 100);
        data_vertex.push_back((float)(rand() % 100) / 100);
        data_vertex.push_back((float)(rand() % 100) / 100);
        data_vertex.push_back(0.0f);
    }

    for (auto face : mesh_faces) {      // Update faces data for rendering
        if (face->render_flag != 1) {
            smfparser::W_edge *e0 = face->edge;
            smfparser::W_edge *edge = e0;
            do {
                data_faces.push_back(vertex_index_map[edge->start]);
                if (edge->left == face)
                    edge = edge->left_next;
                else
                    edge = edge->right_next;
            } while (edge != e0);
        }
    }

    for (auto edge : mesh_edges) {      // Update edges data for rendering
        data_edges.push_back(vertex_index_map[edge.second->start]);
        data_edges.push_back(vertex_index_map[edge.second->end]);
    }
}

} // namespace smfparser
