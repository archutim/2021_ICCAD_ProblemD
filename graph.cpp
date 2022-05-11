#include <iostream>
#include <algorithm>
#include <cfloat>
#include <vector>
#include <math.h>
#include "macro.h"
#include "graph.h"
using namespace std;

extern double chip_width;
extern double chip_height;

// Data structure "adj_matrix" has been commented out.
// Remove // annotation before adj_matrix to use it.

Graph::Graph(int n){
    nodes_num = n;
    g = new vector<edge>[nodes_num + 2];
    g_reversed = new vector<edge>[nodes_num + 2];
    // adj_matrix = new bool *[nodes_num + 2];
    visited = new bool[nodes_num + 2];
    L = new double[nodes_num + 2];
    R = new double[nodes_num + 2];

    for(int i = 0; i < nodes_num + 2; i++){
        // adj_matrix[i] = new bool[nodes_num + 2];
        // for(int j = 0; j < nodes_num + 2; j++){
        //     adj_matrix[i][j] = false;
        // }
        visited[i] = false;
        L[i] = 0;
        R[i] = 0;
    }
}

Graph::~Graph(){
    delete[] g;
    delete[] g_reversed;
    // for(int i = 0; i < nodes_num + 2; i++){
    //     delete[] adj_matrix[i];
    // }
    // delete[] adj_matrix;
    delete[] L;
    delete[] R;
    delete[] visited;
}

void Graph::rebuild(){
    for(int i = 0; i < nodes_num + 2; i++){
        g[i].clear();
        g_reversed[i].clear();
        // for(int j = 0; j < nodes_num + 2; j++){
        //     adj_matrix[i][j] = false;
        // }
        visited[i] = false;
        L[i] = 0.0;
        R[i] = 0.0;
    }
}

void Graph::Copy(Graph& G_copy){
    this->nodes_num = G_copy.nodes_num;
    // Copy Graph and recersed Graph
    for(int i = 0; i < nodes_num + 2; i++){
        for(int j = 0; j < G_copy.g[i].size(); j++){
            this->g[i].push_back(G_copy.g[i][j]);
        }
        for(int j = 0; j < G_copy.g_reversed[i].size(); j++){
            this->g_reversed[i].push_back(G_copy.g_reversed[i][j]);
        }
    }
    // Copy adjacency matrix data structure
    // for (int i = 0; i < nodes_num + 2; i++){
    //     for(int j = 0; j < nodes_num + 2; j++){
    //         adj_matrix[i][j] = G_copy.adj_matrix[i][j];
    //     }
    // }
    // Copy visited and L, R info for each node(macro)
    for (int i = 0; i < nodes_num + 2; i++){
        this->visited[i] = G_copy.visited[i];
        this->L[i] = G_copy.L[i];
        this->R[i] = G_copy.R[i];
    }
}

void Graph::add_edge(int u, int v, double w){
    g[u].push_back(edge(u, v, w));
    g_reversed[v].push_back(edge(u, v, w));
    // adj_matrix[u][v] = true;
}

void Graph::remove_edge(int u, int v){
    for (int i = 0; i < g[u].size(); i++){
        if (g[u][i].to == v){
            g[u].erase(g[u].begin() + i);
            break;
        }
    }
    for (int i = 0; i < g_reversed[v].size(); i++){
        if (g_reversed[v][i].from == u){
            g_reversed[v].erase(g_reversed[v].begin() + i);
            break;
        }
    }
    // adj_matrix[u][v] = false;
}

bool Graph::connected(){
    for(int i = 0; i < nodes_num + 2; i++){
        visited[i] = false;
    }
    vector<int> tp;
    dfs(0, tp);
    bool connected = true;
    for(int i = 0; i < nodes_num + 2; i++){
        if(visited[i] == false)
            connected = false;
    }
    for(int i = 0; i < nodes_num + 2; i++){
        visited[i] = false;
    }

    return connected;
}

// This function is a variation of DFSUtil() in https://www.geeksforgeeks.org/archives/18212
bool Graph::isCyclicUtil(int v, bool *recStack)
{
    if(visited[v] == false)
    {
        // Mark the current node as visited and part of recursion stack
        visited[v] = true;
        recStack[v] = true;
 
        // Recur for all the vertices adjacent to this vertex
        for(int i = 0; i < g[v].size(); i++){
            if(!visited[g[v][i].to] && isCyclicUtil(g[v][i].to, recStack)){
                return true;
            }
            else if(recStack[g[v][i].to]){
                return true;
            }
        }
    }
    recStack[v] = false;  // remove the vertex from recursion stack
    return false;
}
 
// Returns true if the graph contains a cycle, else false.
// This function is a variation of DFS() in https://www.geeksforgeeks.org/archives/18212
bool Graph::isCyclic()
{
    // Mark all the vertices as not visited and not part of recursion
    // stack
    bool *recStack = new bool[nodes_num + 2];
    for(int i = 0; i < nodes_num + 2; i++){
        visited[i] = false;
        recStack[i] = false;
    }
 
    // Call the recursive helper function to detect cycle in different
    // DFS trees
    bool iscycle = false;
    for(int i = 0; i < nodes_num + 2; i++){
        if ( !visited[i] && isCyclicUtil(i, recStack)){
            iscycle = true;
            break;
        }
    }
    for(int i = 0; i < nodes_num + 2; i++){
        visited[i] = false;
    }
    return iscycle;
}


void Graph::dfs(int u, vector<int>& topological_order){
    visited[u] = true;
	for (auto &e : g[u]){
		if (!visited[e.to]){
			dfs(e.to, topological_order);
		}
	}
	topological_order.push_back(u);
}
vector<int> Graph::topological_sort(){
    vector<int> topological_order;
    for (int i = 0; i < nodes_num + 2; i++){
        visited[i] = false;
    }
    for (int i = 0; i < nodes_num + 1; i++){
        if (!visited[i]){
            dfs(i, topological_order);
        }
    }
    
    reverse(topological_order.begin(), topological_order.end());
    return topological_order;
}

double Graph::longest_path(bool is_horizontal){
    vector<int> topological_order = topological_sort();
	for (int i = 0; i <= nodes_num + 1; i++){
		L[i] = 0.0;
		R[i] = DBL_MAX;
	}
	for (auto &u : topological_order){
		for (auto &e : g[u]){
			L[e.to] = max(L[e.to], L[u] + e.weight);
		}
	}
	double chip_boundry;
	if (is_horizontal)
		chip_boundry = chip_width;
	else
		chip_boundry = chip_height;

	R[nodes_num + 1] = max(L[nodes_num + 1], chip_boundry);

	reverse(topological_order.begin(), topological_order.end());

	for (auto &u : topological_order){
		for (auto &e : g_reversed[u]){
			R[e.from] = min(R[e.from], R[u] - e.weight);
		}
	}
	return L[nodes_num + 1];
}

vector<edge> Graph::zero_slack_edges(){
    vector<edge> _zero_slack_edges;
    for(int i = 0; i < nodes_num + 1; i++){
        for(auto &e : g[i]){
            if(round(R[e.to] - L[e.from] - e.weight) == 0){
                _zero_slack_edges.push_back(e);
            }
        }
    }
    return _zero_slack_edges;
}

vector<edge*> Graph::zero_slack(){
    vector<edge*> zero_slack;
    for(int i = 0; i <= nodes_num; i++){
        for(int j = 0; j < g[i].size(); j++){
            if(round(R[g[i][j].to] - L[g[i][j].from] - g[i][j].weight) == 0){
                zero_slack.push_back(&g[i][j]);
            }
        }
    }
    return zero_slack;
}

void Graph::show(vector<Macro*>& macro){
    for (int i = 0; i <= nodes_num; i++){
        if (i == 0)
            cout << "source 's neighbors:\n";
        else
            cout << "macro id" << macro[i - 1]->id() << ' ' << macro[i - 1]->name() << "(is_fixed: " << macro[i - 1]->is_fixed() << ")'s neighbors:\n";
        for (auto &e : g[i])
            cout << "\t" << ((e.to == nodes_num + 1) ? "sink" : macro[e.to - 1]->name()) << " with weight " << e.weight << endl;
        cout << endl;
    }
}

int Graph::get_num(){
    return this->nodes_num;
}

vector<edge>* Graph::get_edge_list(){
    return g;
}

vector<edge>* Graph::get_reverse_edge_list(){
    return g_reversed;
}