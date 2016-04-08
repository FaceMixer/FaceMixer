//////////////////////////////////////////////////////////////////////////////
//
//  match.cc
//
//  The source file for matching algorithm functions
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

#include "match.h"

namespace match {

// The graph used for Dijkstra algorithm
vector<vector<pair<int, int>>> G;

//
// Function: ReadConstrainedVertex
// ---------------------------
//
//   Read the constrained vertex
//
//   Parameters:
//       constrained_vertex: the constrained vertex array
//
//   Returns:
//       void
//

void ReadConstrainedVertex(vector<int> &constrained_vertex) {
    ifstream fin("mesh/index.txt", ifstream::in);

    if (!fin.is_open()) {       // Check if input file exist
        cout << "Index file doesn't exist!" << endl;
        return;
    }

    constrained_vertex.clear();

    int n, vertex_index;
    fin >> n;
    for (int i = 0; i < n; i++) {        // While not end of file
        fin >> vertex_index;
        constrained_vertex.push_back(vertex_index);
    }

    for (auto p: constrained_vertex) {
        cout << p << endl;
    }

    /*
    constrained_vertex.clear();
    constrained_vertex.push_back(1);
    constrained_vertex.push_back(2);
    constrained_vertex.push_back(3);
    constrained_vertex.push_back(4);
    constrained_vertex.push_back(5);
    constrained_vertex.push_back(6);
    constrained_vertex.push_back(7);
    constrained_vertex.push_back(8);
    constrained_vertex.push_back(9);
    constrained_vertex.push_back(10);
    constrained_vertex.push_back(12);
    constrained_vertex.push_back(16);
    constrained_vertex.push_back(17);
    */
}

//
// Function: InitGraph
// ---------------------------
//
//   Init the graph for dijkstra algorithm
//
//   Parameters:
//       void
//
//   Returns:
//       void
//

void InitGraph(vector<smfparser::Vertex *> &match_vertex, map<pair<int, int>, smfparser::W_edge *> &match_edges) {
    for (smfparser::Vertex *v : match_vertex) {             // Generate graph G for dijkstra algorithm
        vector<pair<int, int>> vp;

        smfparser::W_edge *e0 = v->edge;
        smfparser::W_edge *e1;
        if (match_edges.find(make_pair((int)vertex_index_map[v->edge->end] + 1,
                                       (int)vertex_index_map[v->edge->start] + 1)) != match_edges.end()) {
            e1 = match_edges[make_pair((int)vertex_index_map[v->edge->end] + 1,
                                       (int)vertex_index_map[v->edge->start] + 1)];
        } else {
            e1 = nullptr;
        }

        smfparser::W_edge *edge = e0;       // Traverse each edge around a vertex
        do {
            int dist = (int)(sqrtf(powf(edge->start->x - edge->end->x, 2) + powf(edge->start->y - edge->end->y, 2)) * 100000000);
            if (edge->end == v) {
                vp.push_back(make_pair((int)vertex_index_map[edge->start], dist));
                edge = edge->right_prev;
            } else {
                vp.push_back(make_pair((int)vertex_index_map[edge->end], dist));
                edge = edge->left_prev;
            }
        } while (edge != e0 && edge != e1 && edge != nullptr);

        if (edge == nullptr) {              // Boundary edges special judge
            edge = e0;
            do {
                if (edge->end == v) {
                    edge = edge->left_next;
                } else {
                    edge = edge->right_next;
                }
                if (edge != nullptr) {
                    int dist = (int)(sqrtf(powf(edge->start->x - edge->end->x, 2) + powf(edge->start->y - edge->end->y, 2)) * 100000000);
                    vp.push_back(make_pair((int)vertex_index_map[edge->end], dist));
                }
            } while (edge != e0 && edge != e1 && edge != nullptr);
        }

        G.push_back(vp);                    // Get graph G
    }
}

//
// Function: FindShortestPath
// ---------------------------
//
//   Find the shortest path among all pairs of constrained vertex
//
//   Parameters:
//       constrained_vertex: the constrained vertex array
//
//   Returns:
//       void
//

vector<Path *> FindShortestPath(vector<int> &constrained_vertex, map<pair<int, int>, smfparser::W_edge *> &match_edges) {
    vector<Path *> path_array;              // The shortest path result

    for (int i: constrained_vertex) {       // For each constrained vertex as start vertex, do Dijkstra algorithm
        int st = i - 1;
        vector<int> dist(G.size(), libconsts::kMaxPathLength);
        vector<int> prev(G.size(), -1);
        set<pair<int, int>> Q;
        dist[st] = 0;
        Q.insert(make_pair(st, 0));         // Initial start vertex

        while (!Q.empty()) {                // Until no more update
            int v = (*Q.begin()).first;
            Q.erase(Q.begin());

            for (auto it: G[v]) {           // Update from this vertex to all its adjacent vertex
                int v2 = it.first;
                int cost = it.second;
                if (dist[v2] > dist[v] + cost) {        // If find a better path
                    if (dist[v2] != libconsts::kMaxPathLength) {
                        if (Q.find(make_pair(v2, dist[v2])) != Q.end())
                            Q.erase(Q.find(make_pair(v2, dist[v2])));
                    }
                    dist[v2] = dist[v] + cost;          // Update minimum cost
                    if (dist[v2] < 0) exit(EXIT_FAILURE);
                    prev[v2] = v;                       // Update previous vertex
                    Q.insert(make_pair(v2, dist[v2]));
                }
            }
        }

        for (int j: constrained_vertex) {               // Store all pairs of shortest path among constrained vertex
            int ed = j - 1;
            if (st < ed) {
                Path *new_path = new Path(mesh_vertex[st], mesh_vertex[ed], dist[ed]);
                int v1 = ed, v2;
                while (v1 != -1) {
                    v2 = v1;
                    v1 = prev[v1];
                    if (v1 != -1) {
                        new_path->edges.push_back(make_pair(v1 + 1, v2 + 1));
                    }
                }
                reverse(new_path->edges.begin(), new_path->edges.end());
                path_array.push_back(new_path);
            }
        }
    }

    return path_array;
}

//
// Function: CheckLegal
// ---------------------------
//
//   Check if this path is legal to choose
//
//   Parameters:
//       path: the path that need to check
//
//   Returns:
//       void
//

bool CheckLegal(Path *path) {
    if (path->edges.size() != 0)
        return true;
    return false;
}

//
// Function: RecomputeShortestPath
// ---------------------------
//
//   Recompute the shortest path for specific st and ed vertex
//
//   Parameters:
//       void
//
//   Returns:
//       void
//

Path *RecomputeShortestPath(int st, int ed, map<pair<int, int>, smfparser::W_edge *> &match_edges, vector<bool> &deleted_vertex) {
    vector<int> dist(G.size(), libconsts::kMaxPathLength);
    vector<int> prev(G.size(), -1);
    set<pair<int, int>> Q;
    dist[st] = 0;
    Q.insert(make_pair(st, 0));         // Initial start vertex

    while (!Q.empty()) {                // Until no more update
        int v = (*Q.begin()).first;
        Q.erase(Q.begin());

        for (auto it: G[v]) {           // Update from this vertex to all its adjacent vertex
            int v2 = it.first;
            int cost = it.second;
            if ((!deleted_vertex[v2] || v2 == ed) && dist[v2] > dist[v] + cost) {        // If find a better path
                if (dist[v2] != libconsts::kMaxPathLength) {
                    if (Q.find(make_pair(v2, dist[v2])) != Q.end())
                        Q.erase(Q.find(make_pair(v2, dist[v2])));
                }
                dist[v2] = dist[v] + cost;          // Update minimum cost
                prev[v2] = v;                       // Update previous vertex
                Q.insert(make_pair(v2, dist[v2]));
            }
        }
    }

    Path *path = new Path(mesh_vertex[st], mesh_vertex[ed], dist[ed]);      // The shortest path
    int v1 = ed, v2;
    while (v1 != -1) {
        v2 = v1;
        v1 = prev[v1];
        if (v1 != -1) {
            path->edges.push_back(make_pair(v1 + 1, v2 + 1));
        }
    }
    reverse(path->edges.begin(), path->edges.end());

    return path;
}

}  // namespace match
