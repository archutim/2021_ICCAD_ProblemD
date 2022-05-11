#ifndef _FLOW_H_
#define _FLOW_H_

// C++ implementation of Dinic's Algorithm
#include<bits/stdc++.h>
using namespace std;

// A structure to represent a edge between
// two vertex
struct Edge
{
	int v ; // Vertex v (or "to" vertex)
			// of a directed edge u-v. "From"
			// vertex u can be obtained using
			// index in adjacent array.

	double flow ; // flow of data in edge

	double C; // capacity

	int rev ; // To store index of reverse
			// edge in adjacency list so that
			// we can quickly find it.
};


// Residual Graph
class DINIC
{
private:
	int V; // number of vertex
	int *level; // stores level of a node
	int edges_num;
	vector<Edge> *adj;
	vector<pair<int, int>> cut_edges;
public :
	DINIC(int V)
	{
		this->V = V;
		level = new int[V];
		edges_num = 0;
		adj = new vector<Edge>[V];
		cut_edges.clear();
	}

	// Refresh stracture
	void reset(){
		this->V = this->V;
		edges_num = 0;
		for(int i = 0; i < V; i++){
			level[i] = 0;
			adj[i].clear();
		}
		cut_edges.clear();
	}

	// Return # of edges
	int get_edges_num(){
		return edges_num;
	}

	// add edge to the graph
	void addEdge(int u, int v, double C)
	{
		// Forward edge : 0 flow and C capacity
		Edge a{v, 0.0, C, int(adj[v].size())};

		// Back edge : 0 flow and 0 capacity
		Edge b{u, 0.0, 0.0, int(adj[u].size())};
		edges_num++;
		adj[u].push_back(a);
		adj[v].push_back(b); // reverse edge
	}

	vector<pair<int, int>> get_cut_edges(){
		return cut_edges;
	}
	// Finds if more flow can be sent from s to t.
	// Also assigns levels to nodes.
	bool BFS(int s, int t){
		for (int i = 0 ; i < V ; i++)
			level[i] = -1;

		level[s] = 0; // Level of source vertex

		// Create a queue, enqueue source vertex
		// and mark source vertex as visited here
		// level[] array works as visited array also.
		list< int > q;
		q.push_back(s);

		vector<Edge>::iterator i ;
		while (!q.empty())
		{
			int u = q.front();
			q.pop_front();
			for (i = adj[u].begin(); i != adj[u].end(); i++)
			{
				Edge &e = *i;
				if (level[e.v] < 0 && e.flow < e.C)
				{
					// Level of current vertex is,
					// level of parent + 1
					level[e.v] = level[u] + 1;

					q.push_back(e.v);
				}
			}
		}

		// IF we can not reach to the sink we
		// return false else true
		return level[t] < 0 ? false : true ;
	}

	// A DFS based function to send flow after BFS has
	// figured out that there is a possible flow and
	// constructed levels. This function called multiple
	// times for a single call of BFS.
	// flow : Current flow send by parent function call
	// start[] : To keep track of next edge to be explored.
	//		 start[i] stores count of edges explored
	//		 from i.
	// u : Current vertex
	// t : Sink
	double sendFlow(int u, double flow, int t, int start[]){
		// Sink reached
		if (u == t)
			return flow;

		// Traverse all adjacent edges one -by - one.
		for ( ; start[u] < adj[u].size(); start[u]++)
		{
			// Pick next edge from adjacency list of u
			Edge &e = adj[u][start[u]];
										
			if (level[e.v] == level[u]+1 && e.flow < e.C)
			{
				// find minimum flow from u to t
				double curr_flow = min(flow, e.C - e.flow);

				double temp_flow = sendFlow(e.v, curr_flow, t, start);

				// flow is greater than zero
				if (temp_flow > 0)
				{
					// add flow to current edge
					e.flow += temp_flow;

					// subtract flow from reverse edge
					// of current edge
					adj[e.v][e.rev].flow -= temp_flow;
					return temp_flow;
				}
			}
		}

		return 0;
	}

	// Returns maximum flow in graph
	double DinicMaxflow(int s, int t){
		// Corner case
		if (s == t)
			return -1;

		double total = 0; // Initialize result

		// Augment the flow while there is path
		// from source to sink
		while (BFS(s, t) == true)
		{
			// store how many edges are visited
			// from V { 0 to V }
			int *start = new int[V+1] {0};

			// while flow is not zero in graph from S to D
			while (double flow = sendFlow(s, DBL_MAX, t, start))

				// Add path flow to overall flow
				total += flow;
		}

		// return maximum flow
		return total;
	}

	void min_cut_DFS(int u, bool* visit){
		visit[u] = true;
		for(int i = 0; i < adj[u].size(); i++){
			if(!visit[adj[u][i].v] && adj[u][i].flow < adj[u][i].C){
				min_cut_DFS(adj[u][i].v, visit);
			}
		}
	}

	bool min_cut(int s, int t){
		double maximum_flow = 0;
		maximum_flow = DinicMaxflow(s, t);
		bool* visit = new bool[V];
		for(int i = 0; i < V; i++){
			visit[i] = false;
		}
		min_cut_DFS(s, visit);
		double cut_size = 0.0;
		for(int i = 0; i < V; i++){
			if(visit[i]){
				for(int j = 0; j < adj[i].size(); j++){
					if(!visit[adj[i][j].v]/* && adj[i][j].C > 0.0*/){ // I don't know
						cut_edges.push_back(make_pair(i, adj[i][j].v));
						if(adj[i][j].C == DBL_MAX){
							// This case means that min_cut set most contains infinity edges.
							cout << "min cut set contains infinity edge(s)." << endl;
							cut_edges.clear();
							return false;
						}
					}
				}
			}
		}
		delete visit;
		return true;
	}
};

#endif
