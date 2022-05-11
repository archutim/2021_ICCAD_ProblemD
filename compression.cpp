#include <algorithm>
#include "graph.h"
#include "adjustment.h"
#include "compression.h"
#include "flow.h"
#include "constraint_graph.h"

using namespace std;

extern double chip_width, chip_height;
extern double powerplan_width, min_spacing;
extern int macros_num;

void compression_build_Gc(vector<edge>& critical_edges, Graph &G, Graph &G_the_other_dir, DINIC &Gc, vector<Macro*>& macros, double target_value, bool is_G_horizontal)
{
	// maximum longest path(changed to direction) generated when try change macros' relation.
	for (auto &e : critical_edges){
		if (e.from == 0 || e.to == macros_num + 1){
			Gc.addEdge(e.from, e.to, DBL_MAX);
		}
		else{
			// Choose relation between left(bottom) macro and right(top),
			// 						in the direction they would change to.
			// "bottom-top" : change to the new direction, e.from macro will at bottom, e.to at top
			// "top-bottom" : change to the new direction, e.from macro will at top, e.to at bottom
			// "left-right" : change to the new direction, e.from macro will at left, e.to at right
			// "right-left" : change to the new direction, e.from macro will at right, e.to at left
			string relation;
			if(is_G_horizontal){
				if((macros[e.from - 1]->name() == "null" && macros[e.from - 1]->y1() == 0) ||
					(macros[e.to - 1]->name() == "null" && macros[e.to - 1]->y2() == chip_height)){
					relation = "bottom-top";
				}
				else if((macros[e.from - 1]->name() == "null" && macros[e.from - 1]->y2() == chip_height) ||
						(macros[e.to - 1]->name() == "null" && macros[e.to - 1]->y1() == 0)){
					relation = "top-bottom";
				}
				else{
					if(macros[e.from - 1]->cy() == macros[e.to - 1]->cy()){
						if(macros[e.from - 1]->id() < macros[e.to - 1]->id()){
							relation = "bottom-top";
						}
						else{
							relation = "top-bottom";
						}
					}
					else if(macros[e.from - 1]->cy() < macros[e.to - 1]->cy()){
						relation = "bottom-top";
					}
					else{
						relation = "top-bottom";
					}
				}
			}
			else{
				if((macros[e.from - 1]->name() == "null" && macros[e.from - 1]->x1() == 0) ||
					(macros[e.to - 1]->name() == "null" && macros[e.to - 1]->x2() == chip_width)){
					relation = "left-right";
				}
				else if((macros[e.from - 1]->name() == "null" && macros[e.from - 1]->x2() == chip_width) ||
						(macros[e.to - 1]->name() == "null" && macros[e.to - 1]->x1() == 0)){
					relation = "right-left";
				}
				else{
					if(macros[e.from - 1]->cx() == macros[e.to - 1]->cx()){
						if(macros[e.from - 1]->id() < macros[e.to - 1]->id()){
							relation = "left-right";
						}
						else{
							relation = "right-left";
						}
					}
					else if(macros[e.from - 1]->cx() < macros[e.to - 1]->cx()){
						relation = "left-right";
					}
					else{
						relation = "right-left";
					}
				}
			}
			
			// Add edge into G_the_other_dir depends on their relation.
			// After adding edge, calculate new_longest_path and remove the added edge.
			double new_longest_path = 0, edge_weight = 0, penalty = 0;
			if(relation == "bottom-top"){
				if(macros[e.from - 1]->name() == "null" || macros[e.to - 1]->name() == "null"){
					edge_weight = (macros[e.from - 1]->h() + macros[e.to - 1]->h()) / 2;
				}
				else{
					edge_weight = (macros[e.from - 1]->h() + macros[e.to - 1]->h()) / 2 + min_spacing;
				}
				// Store L and R before add edge into the graph of the other direction
				double pre_Ri = G_the_other_dir.R[e.from], pre_Lj = G_the_other_dir.L[e.to];
				G_the_other_dir.add_edge(e.from, e.to, edge_weight);
				new_longest_path = G_the_other_dir.longest_path(false);
				// New L and R after add edge into the graph
				double Li = G_the_other_dir.L[e.from], Ri = G_the_other_dir.R[e.from],
						Lj = G_the_other_dir.L[e.to], Rj = G_the_other_dir.R[e.to];
				double cy_i = macros[e.from - 1]->cy(), cy_j = macros[e.to - 1]->cy();
				if(Li < cy_i && cy_i < Ri && Lj < cy_j && cy_j < Rj &&
					macros[e.from - 1]->y2() - macros[e.to - 1]->y1() < target_value &&
					new_longest_path <= chip_height){
					penalty = (pre_Ri - Ri) + (Lj - pre_Lj) + max(macros[e.from - 1]->y2() - macros[e.to - 1]->y1(), 0.0);
					Gc.addEdge(e.from, e.to, penalty);
				}
				else{
					Gc.addEdge(e.from, e.to, DBL_MAX);
				}
				// remove the added edge
				G_the_other_dir.remove_edge(e.from, e.to);
			}
			else if(relation == "top-bottom"){
				if(macros[e.from - 1]->name() == "null" || macros[e.to - 1]->name() == "null"){
					edge_weight = (macros[e.from - 1]->h() + macros[e.to - 1]->h()) / 2;
				}
				else{
					edge_weight = (macros[e.from - 1]->h() + macros[e.to - 1]->h()) / 2 + min_spacing;
				}
				// Store L and R before add edge into the graph of the other direction
				double pre_Ri = G_the_other_dir.R[e.to], pre_Lj = G_the_other_dir.L[e.from];
				G_the_other_dir.add_edge(e.to, e.from, edge_weight);
				new_longest_path = G_the_other_dir.longest_path(false);
				// New L and R after add edge into the graph
				double Li = G_the_other_dir.L[e.to], Ri = G_the_other_dir.R[e.to],
						Lj = G_the_other_dir.L[e.from], Rj = G_the_other_dir.R[e.from];
				double cy_i = macros[e.to - 1]->cy(), cy_j = macros[e.from - 1]->cy();
				if(Li < cy_i && cy_i < Ri && Lj < cy_j && cy_j < Rj &&
					macros[e.to - 1]->y2() - macros[e.from - 1]->y1() < target_value &&
					new_longest_path <= chip_height){
					penalty = (pre_Ri - Ri) + (Lj - pre_Lj) + max(macros[e.to - 1]->y2() - macros[e.from - 1]->y1(), 0.0);
					Gc.addEdge(e.from, e.to, penalty);
				}
				else{
					Gc.addEdge(e.from, e.to, DBL_MAX);
				}
				// remove the added edge
				G_the_other_dir.remove_edge(e.to, e.from);
			}
			else if(relation == "left-right"){
				if(macros[e.from - 1]->name() == "null" || macros[e.to - 1]->name() == "null"){
					edge_weight = (macros[e.from - 1]->w() + macros[e.to - 1]->w()) / 2;
				}
				else{
					edge_weight = (macros[e.from - 1]->w() + macros[e.to - 1]->w()) / 2 + min_spacing;
				}
				// Store L and R before add edge into the graph of the other direction
				double pre_Ri = G_the_other_dir.R[e.from], pre_Lj = G_the_other_dir.L[e.to];
				G_the_other_dir.add_edge(e.from, e.to, edge_weight);
				new_longest_path = G_the_other_dir.longest_path(true);
				// New L and R after add edge into the graph
				double Li = G_the_other_dir.L[e.from], Ri = G_the_other_dir.R[e.from],
						Lj = G_the_other_dir.L[e.to], Rj = G_the_other_dir.R[e.to];
				double cx_i = macros[e.from - 1]->cx(), cx_j = macros[e.to - 1]->cx();
				if(Li < cx_i && cx_i < Ri && Lj < cx_j && cx_j < Rj &&
					macros[e.from - 1]->x2() - macros[e.to - 1]->x1() < target_value &&
					new_longest_path <= chip_width){
					penalty = (pre_Ri - Ri) + (Lj - pre_Lj) + max(macros[e.from - 1]->x2() - macros[e.to - 1]->x1(), 0.0);
					Gc.addEdge(e.from, e.to, penalty);
				}
				else{
					Gc.addEdge(e.from, e.to, DBL_MAX);
				}
				// remove the added edge
				G_the_other_dir.remove_edge(e.from, e.to);
			}
			else{
				if(macros[e.from - 1]->name() == "null" || macros[e.to - 1]->name() == "null"){
					edge_weight = (macros[e.from - 1]->w() + macros[e.to - 1]->w()) / 2;
				}
				else{
					edge_weight = (macros[e.from - 1]->w() + macros[e.to - 1]->w()) / 2 + min_spacing;
				}
				// Store L and R before add edge into the graph of the other direction
				double pre_Ri = G_the_other_dir.R[e.to], pre_Lj = G_the_other_dir.L[e.from];
				G_the_other_dir.add_edge(e.to, e.from, edge_weight);
				new_longest_path = G_the_other_dir.longest_path(true);
				// New L and R after add edge into the graph
				double Li = G_the_other_dir.L[e.to], Ri = G_the_other_dir.R[e.to],
						Lj = G_the_other_dir.L[e.from], Rj = G_the_other_dir.R[e.from];
				double cx_i = macros[e.to - 1]->cx(), cx_j = macros[e.from - 1]->cx();
				if(Li < cx_i && cx_i < Ri && Lj < cx_j && cx_j < Rj &&
					macros[e.to - 1]->x2() - macros[e.from - 1]->x1() < target_value &&
					new_longest_path <= chip_width){
					penalty = (pre_Ri - Ri) + (Lj - pre_Lj) + max(macros[e.to - 1]->x2() - macros[e.from - 1]->x1(), 0.0);
					Gc.addEdge(e.from, e.to, penalty);
				}
				else{
					Gc.addEdge(e.from, e.to, DBL_MAX);
				}
				// remove the added edge
				G_the_other_dir.remove_edge(e.to, e.from);				
			}
			// Notice may need call G_the_other_dir.longest_path again for recovering it information
			// =======
			G_the_other_dir.longest_path(!is_G_horizontal);
			// =======
		}
	}
}

void Ldfs(vector<edge>& critical_edges, vector<edge>* edge_list, vector<Macro*>& macros, double* L, bool* visited, int u, bool is_horizontal){
	visited[u] = true;
	if(is_horizontal){
		for(int i = 0; i < edge_list[u].size(); i++){
			int node = edge_list[u][i].from;
			double edge_weight = 0;
			if(node == 0){
				edge_weight = macros[u - 1]->w() / 2;
			}
			else if(macros[node - 1]->name() == "null" || macros[u - 1]->name() == "null"){
				edge_weight = (macros[node - 1]->w() + macros[u - 1]->w()) / 2;
			}
			else{
				edge_weight = (macros[node - 1]->w() + macros[u - 1]->w()) / 2 + min_spacing;
			}
			if(L[node] + edge_weight == L[u]){
				critical_edges.push_back(edge(node, u, edge_weight));
				if(!visited[node]){
					Ldfs(critical_edges, edge_list, macros, L, visited, node, is_horizontal);
				}
			}
		}
	}
	else{
		for(int i = 0; i < edge_list[u].size(); i++){
			int node = edge_list[u][i].from;
			double edge_weight = 0;
			if(node == 0){
				edge_weight = macros[u - 1]->h() / 2;
			}
			else if(macros[node - 1]->name() == "null" || macros[u - 1]->name() == "null"){
				edge_weight = (macros[node - 1]->h() + macros[u - 1]->h()) / 2;
			}
			else{
				edge_weight = (macros[node - 1]->h() + macros[u - 1]->h()) / 2 + min_spacing;
			}
			if(L[node] + edge_weight == L[u]){
				critical_edges.push_back(edge(node, u, edge_weight));
				if(!visited[node]){
					Ldfs(critical_edges, edge_list, macros, L, visited, node, is_horizontal);
				}
			}
		}
	}
}

void Rdfs(vector<edge>& critical_edges, vector<edge>* edge_list, vector<Macro*>& macros, double* R, bool* visited, int u, bool is_horizontal){
	visited[u] = true;
	if(is_horizontal){
		for(int i = 0; i < edge_list[u].size(); i++){
			int node = edge_list[u][i].to;
			double edge_weight = 0;
			if(node == macros_num + 1){
				edge_weight = macros[u - 1]->w() / 2;
			}
			else if(macros[node - 1]->name() == "null" || macros[u - 1]->name() == "null"){
				edge_weight = (macros[node - 1]->w() + macros[u - 1]->w()) / 2;
			}
			else{
				edge_weight = (macros[node - 1]->w() + macros[u - 1]->w()) / 2 + min_spacing;
			}
			if(R[node] - edge_weight == R[u]){
				critical_edges.push_back(edge(u, node, edge_weight));
				if(!visited[node]){
					Rdfs(critical_edges, edge_list, macros, R, visited, node, is_horizontal);
				}
			}
		}
	}
	else{
		for(int i = 0; i < edge_list[u].size(); i++){
			int node = edge_list[u][i].to;
			double edge_weight = 0;
			if(node == macros_num + 1){
				edge_weight = macros[u - 1]->h() / 2;
			}
			else if(macros[node - 1]->name() == "null" || macros[u - 1]->name() == "null"){
				edge_weight = (macros[node - 1]->h() + macros[u - 1]->h()) / 2;
			}
			else{
				edge_weight = (macros[node - 1]->h() + macros[u - 1]->h()) / 2 + min_spacing;
			}
			if(R[node] - edge_weight == R[u]){
				critical_edges.push_back(edge(u, node, edge_weight));
				if(!visited[node]){
					Rdfs(critical_edges, edge_list, macros, R, visited, node, is_horizontal);
				}
			}
		}
	}
}

void compression(Graph& Gh, Graph& Gv, vector<Macro*>& macros){
	// Use longest_path to update L and R
	vector<edge>* h_edge_list = Gh.get_edge_list();
	vector<edge>* v_edge_list = Gv.get_edge_list();
	vector<edge>* r_h_edge_list = Gh.get_reverse_edge_list();
	vector<edge>* r_v_edge_list = Gv.get_reverse_edge_list();
	Gh.longest_path(true);
	Gv.longest_path(false);
	bool* visited = new bool[macros_num + 2];
	// if want to fixed (a, b)'s relation after changing once, use bool* fixed
	// bool* fixed = new bool[macros_num];
	vector<edge> critical_edges;
	vector<int> access_order;
	for(int i = 1; i < macros_num + 1; i++){
		access_order.push_back(i);
	}
	random_shuffle(access_order.begin(), access_order.end());
	double target_value = 0;
	for(int i = 0; i < macros_num; i++){
		int now = access_order[i];
		target_value = Gh.L[now] - macros[now - 1]->cx();
		if(target_value > 0){
			memset(visited, false, macros_num + 2);
			Ldfs(critical_edges, r_h_edge_list, macros, Gh.L, visited, now, true);
			DINIC Gc(macros_num + 2);
			compression_build_Gc(critical_edges, Gh, Gv, Gc, macros, target_value, true);
			if(Gc.min_cut(0, now)){
				cout << "compression1!" << endl;
				cut_and_add(Gh, Gv, Gc, macros, true);
				double longest_path_h = Gh.longest_path(true);
				double longest_path_v = Gv.longest_path(false);
			}
			critical_edges.clear();
        }
	}
	random_shuffle(access_order.begin(), access_order.end());
	for(int i = 0; i < macros_num; i++){
		int now = access_order[i];
		target_value = macros[now - 1]->cx() - Gh.R[now];
		if(target_value > 0){
			memset(visited, false, sizeof(bool) * (macros_num + 2));
			Rdfs(critical_edges, h_edge_list, macros, Gh.R, visited, now, true);
			DINIC Gc(macros_num + 2);
			compression_build_Gc(critical_edges, Gh, Gv, Gc, macros, target_value, true);
			if(Gc.min_cut(now, macros_num + 1)){
				cout << "compression2!" << endl;
				cut_and_add(Gh, Gv, Gc, macros, true);
				double longest_path_h = Gh.longest_path(true);
				double longest_path_v = Gv.longest_path(false);
			}
			critical_edges.clear();
		}
	}
	random_shuffle(access_order.begin(), access_order.end());
	for(int i = 0; i < macros_num; i++){
		int now = access_order[i];
		target_value = Gv.L[now] - macros[now - 1]->cy();
		if(target_value > 0){
			memset(visited, false, sizeof(bool) * (macros_num + 2));
			Ldfs(critical_edges, r_v_edge_list, macros, Gv.L, visited, now, false);
			DINIC Gc(macros_num + 2);
			compression_build_Gc(critical_edges, Gv, Gh, Gc, macros, target_value, false);
			if(Gc.min_cut(0, now)){
				cout << "compression3!" << endl;
				cut_and_add(Gv, Gh, Gc, macros, false);
				double longest_path_h = Gh.longest_path(true);
				double longest_path_v = Gv.longest_path(false);
			}
			critical_edges.clear();
		}
	}
	random_shuffle(access_order.begin(), access_order.end());
	for(int i = 0; i < macros_num; i++){
		int now = access_order[i];
		target_value = macros[now - 1]->cy() - Gv.R[now];
		if(target_value > 0){
			memset(visited, false, sizeof(bool) * (macros_num + 2));
			Rdfs(critical_edges, v_edge_list, macros, Gv.R, visited, now, false);
			DINIC Gc(macros_num + 2);
			compression_build_Gc(critical_edges, Gv, Gh, Gc, macros, target_value, false);
			if(Gc.min_cut(now, macros_num + 1)){
				cout << "compression4!" << endl;
				cut_and_add(Gv, Gh, Gc, macros, false);
				double longest_path_h = Gh.longest_path(true);
				double longest_path_v = Gv.longest_path(false);
			}
			critical_edges.clear();
		}
	}
}