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

void ReadConstrainedVertex(vector<int> &constrained_vertex, map<int, pair<float, float>> &constrained_vertex_position) {
    ifstream fin("mesh/index.txt", ifstream::in);

    if (!fin.is_open()) {       // Check if input file exist
        cout << "Index file doesn't exist!" << endl;
        return;
    }

    constrained_vertex.clear();

    int n, vertex_index;
    float x, y;
    fin >> n;
    for (int i = 0; i < n; i++) {        // While not end of file
        fin >> vertex_index >> x >> y;
        constrained_vertex.push_back(vertex_index);
        constrained_vertex_position[vertex_index] = make_pair(x, y);
    }

    fin.close();
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
            int dist = (int)(sqrtf(powf(edge->start->x - edge->end->x, 2) +
                                   powf(edge->start->y - edge->end->y, 2)) * libconsts::kDistanceScalingNumber);
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
                    int dist = (int)(sqrtf(powf(edge->start->x - edge->end->x, 2) +
                                           powf(edge->start->y - edge->end->y, 2)) * libconsts::kDistanceScalingNumber);
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

bool CheckLegal(Path *path, map<int, pair<float, float>> &constrained_vertex_position) {
    if (path->edges.size() == 0) return false;
    int st_index = vertex_index_map[path->st] + 1;
    int ed_index = vertex_index_map[path->ed] + 1;
    glm::vec2 st = glm::vec2(constrained_vertex_position[st_index].first, constrained_vertex_position[st_index].second);
    glm::vec2 ed = glm::vec2(constrained_vertex_position[ed_index].first, constrained_vertex_position[ed_index].second);
    for (auto i: constrained_vertex_position) {
        if (i.first == st_index || i.first == ed_index) continue;
        glm::vec2 point = glm::vec2(i.second.first, i.second.second);
        float dist = (ed.x - st.x) * (point.y - st.y) - (ed.y - st.y) * (point.x - st.x);
        if (fabsf(dist) < 1) continue;
        int t = 0;
        glm::vec2 st2 = glm::vec2(mesh_vertex[st_index - 1]->x, mesh_vertex[st_index - 1]->y);
        glm::vec2 ed2 = glm::vec2(mesh_vertex[ed_index - 1]->x, mesh_vertex[ed_index - 1]->y);
        glm::vec2 point2 = glm::vec2(mesh_vertex[i.first - 1]->x, mesh_vertex[i.first - 1]->y);
        float dist2 = (ed2.x - st2.x) * (point2.y - st2.y) - (ed2.y - st2.y) * (point2.x - st2.x);
        if (fabsf(dist2) < 1) continue;
        if (dist * dist2 >= 0)
            t++;
        else
            t--;
        if (t < 0) {
            //return false;
        }
    }
    return true;
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

// Output match result to file
void OutputPathMatchResult(vector<match::Path *> &TmVc) {
    ofstream fout("mesh/match_result.txt", ofstream::out);
    vector<int> patch(mesh_vertex.size(), -1);
    int patch_number = 1;

    for (auto p : TmVc) {       // Print the matched mesh edges
        for (int i = 0; i < p->edges.size(); i++) {
            if (i == 0) {
                cout << p->edges[i].first << " - " << p->edges[i].second;
                patch[p->edges[i].first - 1] = 0;
                patch[p->edges[i].second - 1] = 0;
            } else {
                cout << " - " << p->edges[i].second;
                patch[p->edges[i].second - 1] = 0;
            }
        }
        cout << endl;
    }

    for (int i = 0; i < mesh_vertex.size(); i++) {              // For each vertex, do floodfill if it is not visited
        if (patch[i] == -1) {
            queue<int> Q;
            vector<int> patch_vertex;
            patch_vertex.clear();
            Q.push(i);                      // Init first vertex in queue
            patch[i] = patch_number;
            patch_vertex.push_back(i);
            while (!Q.empty()) {
                int v = Q.front();
                Q.pop();
                for (auto it : G[v]) {
                    if (patch[it.first] == -1) {        // Floodfill
                        Q.push(it.first);
                        patch[it.first] = patch_number;     // Update patch number for that vertex
                        patch_vertex.push_back(it.first);
                    } else if (patch[it.first] == 0) {      // Find boundary for that patch

                    }
                }
            }
            fout << patch_vertex.size() << endl;
            //for (auto v : patch_vertex) {
            //    fout << v + 1 << endl;
            //}
            fout << endl;
            patch_number++;
        }
    }

    fout.close();
}

}  // namespace match
