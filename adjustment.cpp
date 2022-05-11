#include <algorithm>
#include "graph.h"
#include "adjustment.h"
#include "flow.h"
#include "constraint_graph.h"

using namespace std;

extern double chip_width, chip_height;
extern double powerplan_width, min_spacing;
extern int macros_num;

// Warning: build_Gc function still has problem that add edges in wrong direction since method of determining relation is diff from build_init_CG.
double build_Gc(vector<edge>& critical_edges, Graph &G, Graph &G_the_other_dir, DINIC &Gc, vector<Macro*>& macros, bool is_G_horizontal)
{
	// maximum longest path(changed to direction) generated when try change macros' relation.
	double maximum_path = 0;
	int count = 0;
	for (auto &e : critical_edges){
		if (e.from == 0 || e.to == macros_num + 1){
			Gc.addEdge(e.from, e.to, DBL_MAX);
			count++;
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

			// Need store Rj and Li information before add edge into constraint graph
			double Rj = 0, Li = 0;
			if(relation == "bottom-top" || relation == "left-right"){
				Rj = G_the_other_dir.R[e.to];
				Li = G_the_other_dir.L[e.from];
			}
			else{
				Rj = G_the_other_dir.R[e.from];
				Li = G_the_other_dir.L[e.to];
			}
			
			// Add edge into G_the_other_dir depends on their relation.
			// After adding edge, calculate new_longest_path and remove the added edge.
			double new_longest_path = 0;
			double edge_weight = 0;
			if(relation == "bottom-top"){
				if(macros[e.from - 1]->name() == "null" || macros[e.to - 1]->name() == "null"){
					edge_weight = (macros[e.from - 1]->h() + macros[e.to - 1]->h()) / 2;
				}
				else{
					edge_weight = (macros[e.from - 1]->h() + macros[e.to - 1]->h()) / 2 + min_spacing;
				}
				G_the_other_dir.add_edge(e.from, e.to, edge_weight);
				new_longest_path = G_the_other_dir.longest_path(false);
				G_the_other_dir.remove_edge(e.from, e.to);
			}
			else if(relation == "top-bottom"){
				if(macros[e.from - 1]->name() == "null" || macros[e.to - 1]->name() == "null"){
					edge_weight = (macros[e.from - 1]->h() + macros[e.to - 1]->h()) / 2;
				}
				else{
					edge_weight = (macros[e.from - 1]->h() + macros[e.to - 1]->h()) / 2 + min_spacing;
				}
				G_the_other_dir.add_edge(e.to, e.from, edge_weight);
				new_longest_path = G_the_other_dir.longest_path(false);
				G_the_other_dir.remove_edge(e.to, e.from);
			}
			else if(relation == "left-right"){
				if(macros[e.from - 1]->name() == "null" || macros[e.to - 1]->name() == "null"){
					edge_weight = (macros[e.from - 1]->w() + macros[e.to - 1]->w()) / 2;
				}
				else{
					edge_weight = (macros[e.from - 1]->w() + macros[e.to - 1]->w()) / 2 + min_spacing;
				}
				G_the_other_dir.add_edge(e.from, e.to, edge_weight);
				new_longest_path = G_the_other_dir.longest_path(true);
				G_the_other_dir.remove_edge(e.from, e.to);
			}
			else{
				if(macros[e.from - 1]->name() == "null" || macros[e.to - 1]->name() == "null"){
					edge_weight = (macros[e.from - 1]->w() + macros[e.to - 1]->w()) / 2;
				}
				else{
					edge_weight = (macros[e.from - 1]->w() + macros[e.to - 1]->w()) / 2 + min_spacing;
				}
				G_the_other_dir.add_edge(e.to, e.from, edge_weight);
				new_longest_path = G_the_other_dir.longest_path(true);
				G_the_other_dir.remove_edge(e.to, e.from);

			}
			// Notice may need call G_the_other_dir.longest_path again for recovering it information
			// =======
			G_the_other_dir.longest_path(!is_G_horizontal);
			// =======
			if((is_G_horizontal && new_longest_path > chip_height) ||
					(!is_G_horizontal && new_longest_path > chip_width)){
				Gc.addEdge(e.from, e.to, DBL_MAX);
				count++;

				// Use maximum path to extend chip boundary
				// 		when all relation changing would cause exceed chip boundary 
				if(new_longest_path > maximum_path){
					maximum_path = new_longest_path;
				}
			}
			else{
				double avg_length = 0, edge_weight = 0;
				if(relation == "bottom-top"){	// from => i, to => j 
					if(macros[e.from - 1]->name() == "null" || macros[e.to - 1]->name() == "null"){
						avg_length = (macros[e.from - 1]->h() + macros[e.to - 1]->h()) / 2;
					}
					else{
						avg_length = (macros[e.from - 1]->h() + macros[e.to - 1]->h()) / 2 + min_spacing;
					}
					edge_weight = max(macros[e.from - 1]->cy() + avg_length - Rj, 0.0)
											+ max(Li + avg_length - macros[e.to - 1]->cy(), 0.0);
					Gc.addEdge(e.from, e.to, edge_weight);
				}
				else if(relation == "top-bottom"){	// from => j, to => i
					if(macros[e.from - 1]->name() == "null" || macros[e.to - 1]->name() == "null"){
						avg_length = (macros[e.from - 1]->h() + macros[e.to - 1]->h()) / 2;
					}
					else{
						avg_length = (macros[e.from - 1]->h() + macros[e.to - 1]->h()) / 2 + min_spacing;
					}
					edge_weight = max(macros[e.to - 1]->cy() + avg_length - Rj, 0.0)
											+ max(Li + avg_length - macros[e.from - 1]->cy(), 0.0);
					Gc.addEdge(e.from, e.to, edge_weight);
				}
				else if(relation == "left-right"){	// from => i, to => j 
					if(macros[e.from - 1]->name() == "null" || macros[e.to - 1]->name() == "null"){
						avg_length = (macros[e.from - 1]->w() + macros[e.to - 1]->w()) / 2;
					}
					else{
						avg_length = (macros[e.from - 1]->w() + macros[e.to - 1]->w()) / 2 + min_spacing;
					}
					edge_weight = max(macros[e.from - 1]->cy() + avg_length - Rj, 0.0)
											+ max(Li + avg_length - macros[e.to - 1]->cy(), 0.0);
					Gc.addEdge(e.from, e.to, edge_weight);
				}
				else{	// from => j, to => i
					if(macros[e.from - 1]->name() == "null" || macros[e.to - 1]->name() == "null"){
						avg_length = (macros[e.from - 1]->w() + macros[e.to - 1]->w()) / 2;
					}
					else{
						avg_length = (macros[e.from - 1]->w() + macros[e.to - 1]->w()) / 2 + min_spacing;
					}
					edge_weight = max(macros[e.to - 1]->cy() + avg_length - Rj, 0.0)
											+ max(Li + avg_length - macros[e.from - 1]->cy(), 0.0);
					Gc.addEdge(e.from, e.to, edge_weight);
				}
			}
		}
	}
	// use maximum_path as reference for ectending chip boundary
	return maximum_path;
}

void cut_and_add(Graph &G, Graph &G_the_other_dir, DINIC &Gc, vector<Macro*>& macros, bool is_G_horizontal){
	vector<pair<int, int>> cut_edges = Gc.get_cut_edges();
	vector<edge>* G_edge_list = G.get_edge_list();
	vector<edge>* r_G_edge_list = G.get_reverse_edge_list();
	// To prevent the set "cut_edges" contains edges connecting to source or sink
	for(int i = 0; i < cut_edges.size(); i++){
		if(cut_edges[i].first == 0 || cut_edges[i].second == macros_num + 1){
			cout << "Drop the cut edge connect to source(sink)" << endl;
			cut_edges.erase(cut_edges.begin() + i);
		}
	}
	if(cut_edges.empty()){
		cout << "Failed when doing min-cut (No cut edge)" << endl;
		return;
	}
	// Since some cases cause DINIC Algo catching reverse edge(s) as min-cut element
	// In order to prevent these cases cause cycle, we need to find out it and repair it.
	for(auto &e : cut_edges){
		for(int i = 0; i < r_G_edge_list[e.first].size(); i++){
				if(r_G_edge_list[e.first][i].from == e.second){
					int temp = e.first;
					e.first = e.second;
					e.second = temp;
					break;
				}
		}
	}

	// Notice: In our implementation, we add patch edges and remove min-cut edges alternatively.
	// 				Instead of adding patch edges after remove all min-cut edges.
	// This may cause errors, or just make the graph contains some redundent edges ?
	if(is_G_horizontal){
		for (auto &e : cut_edges){
			G.remove_edge(e.first, e.second);
			// Ensure graph G would still be connected after cutting edges.
			// We add patch edges in following part
			// ============================================================
			for(int i = 0; i < r_G_edge_list[e.first].size(); i++){
				if(r_G_edge_list[e.first][i].from == 0){
					G.add_edge(0, e.second, macros[e.second - 1]->w() / 2);
				}
				else if(macros[r_G_edge_list[e.first][i].from - 1]->name() == "null" || macros[e.second - 1]->name() == "null"){
					double edge_weight = (macros[r_G_edge_list[e.first][i].from - 1]->w() + macros[e.second - 1]->w()) / 2;
					G.add_edge(r_G_edge_list[e.first][i].from, e.second, edge_weight);
				}
				else{
					double edge_weight = (macros[r_G_edge_list[e.first][i].from - 1]->w() + macros[e.second - 1]->w()) / 2 + min_spacing;
					G.add_edge(r_G_edge_list[e.first][i].from, e.second, edge_weight);
				}
			}
			for(int i = 0; i < G_edge_list[e.second].size(); i++){
				if(G_edge_list[e.second][i].to == macros_num + 1){
					G.add_edge(e.first, macros_num + 1, macros[e.first - 1]->w() / 2);
				}
				else if(macros[e.first - 1]->name() == "null" || macros[G_edge_list[e.second][i].to - 1]->name() == "null"){
					double edge_weight = (macros[e.first - 1]->w() + macros[G_edge_list[e.second][i].to - 1]->w()) / 2;
					G.add_edge(e.first, G_edge_list[e.second][i].to, edge_weight);
				}
				else{
					double edge_weight = (macros[e.first - 1]->w() + macros[G_edge_list[e.second][i].to - 1]->w()) / 2 + min_spacing;
					G.add_edge(e.first, G_edge_list[e.second][i].to, edge_weight);
				}
			}
			// ============================================================
			double edge_weight = 0;
			if(macros[e.first - 1]->name() == "null"){
				edge_weight = (macros[e.first - 1]->h() + macros[e.second - 1]->h()) / 2;
				if(macros[e.first - 1]->y1() == 0){
					G_the_other_dir.add_edge(e.first, e.second, edge_weight);
				}
				else if(macros[e.first - 1]->y2() == chip_height){
					G_the_other_dir.add_edge(e.second, e.first, edge_weight);
				}
			}
			else if(macros[e.second - 1]->name() == "null"){
				edge_weight = (macros[e.first - 1]->h() + macros[e.second - 1]->h()) / 2;
				if(macros[e.second - 1]->y1() == 0){
					G_the_other_dir.add_edge(e.second, e.first, edge_weight);
				}
				else if(macros[e.second - 1]->y2() == chip_height){
					G_the_other_dir.add_edge(e.first, e.second, edge_weight);
				}
			}
			else{
				edge_weight = (macros[e.first - 1]->h() + macros[e.second - 1]->h()) / 2 + min_spacing;
				if(macros[e.first - 1]->cy() == macros[e.second - 1]->cy()){
					if(macros[e.first - 1]->id() < macros[e.second - 1]->id()){
						G_the_other_dir.add_edge(e.first, e.second, edge_weight);
					}
					else{
						G_the_other_dir.add_edge(e.second, e.first, edge_weight);
					}
				}
				else if(macros[e.first - 1]->cy() < macros[e.second - 1]->cy()){
					G_the_other_dir.add_edge(e.first, e.second, edge_weight);
				}
				else{
					G_the_other_dir.add_edge(e.second, e.first, edge_weight);
				}
			}
		}
	}
	else{
		for (auto &e : cut_edges){
			G.remove_edge(e.first, e.second);
			// Ensure graph G would still be connected after cutting edges.
			// We will add patch edges in following part.
			// ============================================================
			for(int i = 0; i < r_G_edge_list[e.first].size(); i++){
				if(r_G_edge_list[e.first][i].from == 0){
					G.add_edge(0, e.second, macros[e.second - 1]->h() / 2);
				}
				else if(macros[r_G_edge_list[e.first][i].from - 1]->name() == "null" || macros[e.second - 1]->name() == "null"){
					double edge_weight = (macros[r_G_edge_list[e.first][i].from - 1]->h() + macros[e.second - 1]->h()) / 2;
					G.add_edge(r_G_edge_list[e.first][i].from, e.second, edge_weight);
				}
				else{
					double edge_weight = (macros[r_G_edge_list[e.first][i].from - 1]->h() + macros[e.second - 1]->h()) / 2 + min_spacing;
					G.add_edge(r_G_edge_list[e.first][i].from, e.second, edge_weight);
				}
			}
			for(int i = 0; i < G_edge_list[e.second].size(); i++){
				if(G_edge_list[e.second][i].to == macros_num + 1){
					G.add_edge(e.first, macros_num + 1, macros[e.first - 1]->h() / 2);
				}
				else if(macros[e.first - 1]->name() == "null" || macros[G_edge_list[e.second][i].to - 1]->name() == "null"){
					double edge_weight = (macros[e.first - 1]->h() + macros[G_edge_list[e.second][i].to - 1]->h()) / 2;
					G.add_edge(e.first, G_edge_list[e.second][i].to, edge_weight);
				}
				else{
					double edge_weight = (macros[e.first - 1]->h() + macros[G_edge_list[e.second][i].to - 1]->h()) / 2 + min_spacing;
					G.add_edge(e.first, G_edge_list[e.second][i].to, edge_weight);
				}
			}
			// ============================================================
			double edge_weight = 0;
			if(macros[e.first - 1]->name() == "null"){
				edge_weight = (macros[e.first - 1]->w() + macros[e.second - 1]->w()) / 2;
				if(macros[e.first - 1]->x1() == 0){
					G_the_other_dir.add_edge(e.first, e.second, edge_weight);
				}
				else if(macros[e.first - 1]->x2() == chip_width){
					G_the_other_dir.add_edge(e.second, e.first, edge_weight);
				}
			}
			else if(macros[e.second - 1]->name() == "null"){
				edge_weight = (macros[e.first - 1]->w() + macros[e.second - 1]->w()) / 2;
				if(macros[e.second - 1]->x1() == 0){
					G_the_other_dir.add_edge(e.second, e.first, edge_weight);
				}
				else if(macros[e.second - 1]->x2() == chip_width){
					G_the_other_dir.add_edge(e.first, e.second, edge_weight);
				}
			}
			else{
				edge_weight = (macros[e.first - 1]->w() + macros[e.second - 1]->w()) / 2 + min_spacing;
				if(macros[e.first - 1]->cx() == macros[e.second - 1]->cx()){
					if(macros[e.first - 1]->id() < macros[e.second - 1]->id()){
						G_the_other_dir.add_edge(e.first, e.second, edge_weight);
					}
					else{
						G_the_other_dir.add_edge(e.second, e.first, edge_weight);
					}
				}
				else if(macros[e.first - 1]->cx() < macros[e.second - 1]->cx()){
					G_the_other_dir.add_edge(e.first, e.second, edge_weight);
				}
				else{
					G_the_other_dir.add_edge(e.second, e.first, edge_weight);
				}
			}
		}
	}
}

bool adjustment(Graph &Gh, Graph &Gv, vector<Macro*>& macros, bool init)
{
	double longest_path_h = 0;
	double longest_path_v = 0;
	int priority = (init) ? INT_MAX : 30;
	while(priority--){
		longest_path_h = Gh.longest_path(true);
		longest_path_v = Gv.longest_path(false);
		// case1 : Both constraint graphs are legal
		if(longest_path_h <= chip_width && longest_path_v <= chip_height){
			break;
		}
		// case2 : Both constraint graphs are illegal
		// else if(longest_path_h > chip_width && longest_path_v > chip_height){
		// ...
		// }
		// case3 : Horizontal constraint graph is illegal
		else if(longest_path_h > chip_width){
			// declare a graph for min-cut and initialize its nodes number
			DINIC Gc(macros_num + 2);
			vector<edge> critical_edges = Gh.zero_slack_edges();
			double result = build_Gc(critical_edges, Gh, Gv, Gc, macros, true);
			double buffer = macros[rand() % macros_num]->h();
			while(!Gc.min_cut(0, macros_num + 1)){
				// save origin chip_height
				double saved_height = chip_height;
				// enlarge chip_height
				chip_height = result + buffer;
				buffer += buffer;
				// reset Gc
				Gc.reset();
				// Re-run build-Gc
				result = build_Gc(critical_edges, Gh, Gv, Gc, macros, true);
				chip_height = saved_height;
				cout << "Gh to Gv, All edges in Gc have weight infinity." << endl;
				// Need to something to deal with it.
			}
			cut_and_add(Gh, Gv, Gc, macros, true);
		}
		// case4 : Vertical constraint graph is illegal
		else{
			// declare a graph for min-cut and initialize its nodes number
			DINIC Gc(macros_num + 2);
			vector<edge> critical_edges = Gv.zero_slack_edges();
			double result = build_Gc(critical_edges, Gv, Gh, Gc, macros, false);
			double buffer = macros[rand() % macros_num]->w();
			while(!Gc.min_cut(0, macros_num + 1)){
				// save origin chip_width
				double saved_width = chip_width;
				// enlarge chip_width
				chip_width = result + buffer;
				buffer += buffer;
				// reset Gc
				Gc.reset();
				// Re-run build-Gc
				result = build_Gc(critical_edges, Gv, Gh, Gc, macros, false);
				chip_width = saved_width;
				cout << "Gv to Gh, All edges in Gc have weight infinity." << endl;
				// Need to something to deal with it.
			}
			cut_and_add(Gv, Gh, Gc, macros, false);
		}
	}
	if(longest_path_h > chip_width || longest_path_v > chip_height){
		return false;
	}
	else{
		return true;
	}
}