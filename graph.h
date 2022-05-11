#ifndef _GRAPH_H_
#define _GRAPH_H_

#include <iostream>
#include <vector>
#include "macro.h"

using namespace std;

extern double chip_width;
extern double chip_height;
extern Macro **macros;

class edge{

public:
	int from, to;
	double weight;
	edge(int u, int v, double w){
		from = u;
		to = v;
		weight = w;
	}
};

class Graph{

private:
	int nodes_num; //exclude source and sink. => equals to "macros_num"
	vector<edge>* g;
	vector<edge>* g_reversed;
	bool* visited;
public:
	// bool** adj_matrix;
	double *L, *R;

	Graph(int n);
	~Graph();
	void rebuild();
	void Copy(Graph& G_copy);
	void add_edge(int u, int v, double w);
	void remove_edge(int u, int v);
	bool connected();
	bool isCyclicUtil(int v, bool *recStack);
	bool isCyclic();
	void dfs(int u, vector<int>& topological_order);
	vector<int> topological_sort();
	// Notice: Every time calls longest_path function,
	// 			 it will calculate instead of returning a value directly.
	// O(V + E)
	double longest_path(bool is_horizontal);
	// O(V + E)
	vector<edge> zero_slack_edges();
	vector<edge*> zero_slack();
	void show(vector<Macro*>& macro);
	vector<int> L_forced_displace(bool is_horizontal);
	vector<int> R_forced_displace(bool is_horizontal);
	int get_num();
	vector<edge>* get_edge_list();
	vector<edge>* get_reverse_edge_list();
};

#endif
