#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <math.h>
#include "macro.h"
#include "graph.h"
#include "evaluation.h"
#include "transitive_reduction.h"
#include "corner_stitch/utils/update.h"

extern double chip_width, chip_height, min_spacing, buffer_constraint, powerplan_width, alpha, beta;
extern int micron;
extern int macros_num;

static int collectTiles(Tile *tile, ClientData cdata){
	((vector<Tile*>*) cdata)->push_back(tile);
	return 0;
}

static int buffer_constraint_check(Tile *tile, ClientData cdata){
	if(TiGetBody(tile) != SOLID_TILE){
		*((bool*)cdata) = true;
	}
	return 0;
}

static int collect_neighbor_macro(Tile *tile, ClientData cdata){
	if(TiGetBody(tile) == SOLID_TILE){
		((vector<int>*) cdata)->push_back(TiGetClient(tile));
	}
	return 0;
}

bool Horizontal_update(Plane* horizontal_plane, Plane* vertical_plane,
						Rect& horizontal_region, Rect& vertical_region, double& powerplan_cost){
	// Use boolean for checking if any tiles are inserted in this round
	bool updated = false;

	// Vector stores tiles(in horizontal corner stitch data structure) have overlap with placement region
	vector<Tile*> horizontal_tiles;

	// Find tiles which have overlap with placement region
	TiSrArea(NULL, horizontal_plane, &horizontal_region, collectTiles, (ClientData)&horizontal_tiles);

	for(int i = 0; i < horizontal_tiles.size(); i++){

		// If the tile is solid, means that it is macro or cost_area. We don't need to care if it would become cost_area.
		if(TiGetBody(horizontal_tiles[i]) == SOLID_TILE)
			continue;

		// Decides left, right boundary of tile
		double left = LEFT(horizontal_tiles[i]), right = RIGHT(horizontal_tiles[i]);
		if(left < 0)	left = 0;
		if(right > chip_width) right = chip_width;

		// If tile's width is smaller than powerplan_width, makes the tile become cost_area
		if(right - left < powerplan_width){
			Rect horizontal_cost_tile = { {left, BOTTOM(horizontal_tiles[i])}, {right, TOP(horizontal_tiles[i])} };
			if (InsertTile(&horizontal_cost_tile, horizontal_plane, SOLID_TILE, -1) == NULL) {
				printf("(Horizontal)Invalid insertion due to the overlapping with existing rectangles: (%lf, %lf) - (%lf, %lf).\n",
						left, BOTTOM(horizontal_tiles[i]), right, TOP(horizontal_tiles[i]));
			}

			// Needs to add corresponding tile into vertical corner_stitch data structure at the same time
			Rect vertical_cost_tile = { {-TOP(horizontal_tiles[i]), left}, {-BOTTOM(horizontal_tiles[i]), right} };
			if (InsertTile(&vertical_cost_tile, vertical_plane, SOLID_TILE, -1) == NULL) {
				printf("(Vertical)Invalid insertion due to the overlapping with existing rectangles: (%lf, %lf) - (%lf, %lf).\n",
						-TOP(horizontal_tiles[i]), left, -BOTTOM(horizontal_tiles[i]), right);
			}

			// Update powerplan_cost
			powerplan_cost += ((double)(right - left) / (double)micron) * ((double)(TOP(horizontal_tiles[i]) - BOTTOM(horizontal_tiles[i])) / (double)micron);
			
			// There are some tiles are inserted in this round
			updated = true;
		}
	}
	return updated;
}

bool Vertical_update(Plane* horizontal_plane, Plane* vertical_plane,
						Rect& horizontal_region, Rect& vertical_region, double& powerplan_cost){
	// Use boolean for checking if any tiles are inserted in this round
	bool updated = false;

	// Vector stores tiles(in vertical corner stitch data structure) have overlap with placement region
	vector<Tile*> vertical_tiles;

	// Find tiles which have overlap with placement region
	TiSrArea(NULL, vertical_plane, &vertical_region, collectTiles, (ClientData)&vertical_tiles);

	for(int i = 0; i < vertical_tiles.size(); i++){

		// If the tile is solid, means that it is macro or cost_area. We don't need to care if it would become cost_area.		
		if(TiGetBody(vertical_tiles[i]) == SOLID_TILE)
			continue;

		// Decides left, right boundary of tile
		double left = LEFT(vertical_tiles[i]), right = RIGHT(vertical_tiles[i]);
		if(left < -chip_height)	left = -chip_height;
		if(right > 0) right = 0;

		// If tile's width is smaller than powerplan_width, makes the tile become cost_area
		if(right - left < powerplan_width){
			// Needs to add corresponding tile into horizontal corner_stitch data structure at the same time
			Rect horizontal_cost_tile = { {BOTTOM(vertical_tiles[i]), -right}, {TOP(vertical_tiles[i]), -left} };
			if (InsertTile(&horizontal_cost_tile, horizontal_plane, SOLID_TILE, -1) == NULL) {
				printf("(Horizontal)Invalid insertion due to the overlapping with existing rectangles: (%lf, %lf) - (%lf, %lf).\n",
						BOTTOM(vertical_tiles[i]), -right, TOP(vertical_tiles[i]), -left);
			}		
			Rect vertical_cost_tile = { {left, BOTTOM(vertical_tiles[i])}, {right, TOP(vertical_tiles[i])} };
			if (InsertTile(&vertical_cost_tile, vertical_plane, SOLID_TILE, -1) == NULL) {
				printf("(Vertical)Invalid insertion due to the overlapping with existing rectangles: (%lf, %lf) - (%lf, %lf).\n",
						left, BOTTOM(vertical_tiles[i]), right, TOP(vertical_tiles[i]));
			}

			// Update powerplan_cost
			powerplan_cost += ((double)(right - left) / (double)micron) * ((double)(TOP(vertical_tiles[i]) - BOTTOM(vertical_tiles[i])) / (double)micron);

			// There are some tiles are inserted in this round
			updated = true;
		}
	}
	return updated;
}

double cost_evaluation(vector<Macro*>& macro, Plane* horizontal_plane, Plane* vertical_plane)
{

	// powerplan_cost
	double powerplan_cost = 0;

	// Insert macros into plane
	for(int i = 0; i < macro.size(); i++){

		// In horizontal (left, bottom), (right, top) = (x1, y1), (x2, y2)
		Rect horizontal_rect = { {macro[i]->x1(), macro[i]->y1()}, {macro[i]->x2(), macro[i]->y2()} };
		// In vertical (left, bottom), (right, top) = (-y2, x1), (-y1, x2)
		Rect vertical_rect = { {-macro[i]->y2(), macro[i]->x1()}, {-macro[i]->y1(), macro[i]->x2()} };

		if (InsertTile(&horizontal_rect, horizontal_plane, SOLID_TILE, i) == NULL) {
			printf("(Horizontal)Invalid insertion due to the overlapping with existing rectangles: (%lf, %lf) - (%lf, %lf).\n",
					macro[i]->x1(), macro[i]->y1(), macro[i]->x2(), macro[i]->y2());
		}
		if (InsertTile(&vertical_rect, vertical_plane, SOLID_TILE, i) == NULL) {
			printf("(Vertical)Invalid insertion due to the overlapping with existing rectangles: (%lf, %lf) - (%lf, %lf).\n",
					-macro[i]->y2(), macro[i]->x1(), -macro[i]->y1(), macro[i]->x2());
		}
	}

	// Rect horizontal(vertical)_region represents place region
	Rect horizontal_region = { {0, 0}, {chip_width, chip_height} };
	Rect vertical_region = { {-chip_height, 0}, {0, chip_width} };

	// Update corner stitch data structure until all cost_area are found
	bool horizontal_updated = true, vertical_updated = true;
	while(horizontal_updated && vertical_updated){
		horizontal_updated = false;
		vertical_updated = false;
		horizontal_updated = Horizontal_update(horizontal_plane, vertical_plane, horizontal_region, vertical_region, powerplan_cost);
		vertical_updated = Vertical_update(horizontal_plane, vertical_plane, horizontal_region, vertical_region, powerplan_cost);
	}

	// Return powerplan cost
	return powerplan_cost;
}

vector<int> invalid_check(vector<Macro*>& macro, Plane* horizontal_plane){

	// Vector stores invalid macros found by corner stitch data structure
	vector<int> invalid_macros;
	for(int i = 0; i < macro.size(); i++){
		if(macro[i]->name() == "null")
			continue;
		// Determine search area boundary for macro
		double left = macro[i]->x1() - buffer_constraint,
			right = macro[i]->x2() + buffer_constraint,
			top = macro[i]->y2() + buffer_constraint,
			bottom = macro[i]->y1() - buffer_constraint;
		if(left < 0) left = 0;
		if(right > chip_width) right = chip_width;
		if(top > chip_height) top = chip_height;
		if(bottom < 0) bottom = 0;

		// Create search area
		Rect macro_area = { {left, bottom}, {right, top} };

		// Use boolean for checking if this macro is valid
		bool valid = false;

		// Find tiles have overlap with search area, if there are any tile is space tile then the macro is valid
		TiSrArea(NULL, horizontal_plane, &macro_area, buffer_constraint_check, (ClientData)&valid);

		if(!valid){
			invalid_macros.push_back(i);
		}
	}

	// Show all invalid macros
	if(!invalid_macros.empty()){
		printf("Invalid macros:\n");
	}
	for(int i = 0; i < invalid_macros.size(); i++){
		cout << "macro " << macro[invalid_macros[i]]->name() << " is invalid" << endl;
	}

	return invalid_macros;
}

void fix_invalid(vector<Macro*>& macro, vector<int>& invalid_macros, Graph& Gh, Graph& Gv){

	// Read edges list
	vector<edge>* h_edge_list = Gh.get_edge_list();
	vector<edge>* v_edge_list = Gv.get_edge_list();
	vector<edge>* r_h_edge_list = Gh.get_reverse_edge_list();
	vector<edge>* r_v_edge_list = Gv.get_reverse_edge_list();

	// Use a table for recording potential edges which 
	map<edge*, int> edges_table;
	for(int i = 0; i < invalid_macros.size(); i++){
		// Set [buffer + powerplan] boundary of invalid macro
		double left = macro[invalid_macros[i]]->x1() - powerplan_width,
			right = macro[invalid_macros[i]]->x2() + powerplan_width,
			top = macro[invalid_macros[i]]->y2() + powerplan_width,
			bottom = macro[invalid_macros[i]]->y1() - powerplan_width;
		if(left < 0) left = 0;
		if(right > chip_width) right = chip_width;
		if(top > chip_height) top = chip_height;
		if(bottom < 0) bottom = 0;

		// Horizontal_edge : invalid -> neighbor
		for(int j = 0; j < h_edge_list[invalid_macros[i] + 1].size(); j++){
			// .to can't be sink
			if(h_edge_list[invalid_macros[i] + 1][j].to != macro.size() + 1){
				// right neighbor overlaps with region created by invalid_macro
				if(macro[h_edge_list[invalid_macros[i] + 1][j].to - 1]->x1() < right){
					// If the edge hasn't been found, add it into global table
					map<edge*, int>::iterator it;
					it = edges_table.find(&h_edge_list[invalid_macros[i] + 1][j]);
					if(it != edges_table.end()){
						it->second ++;
					}
					else{
						edges_table.insert(make_pair(&h_edge_list[invalid_macros[i] + 1][j], 1));
					}
				}
			}
		}
		// Horizontal_edge : neighbor -> invalid
		for(int j = 0; j < r_h_edge_list[invalid_macros[i] + 1].size(); j++){
			// .from can't be source
			if(r_h_edge_list[invalid_macros[i] + 1][j].from != 0){
				// left neighbor overlaps with region created by invalid_macro
				if(macro[r_h_edge_list[invalid_macros[i] + 1][j].from - 1]->x2() > left){
					// If the edge hasn't been found, add it into global table
					map<edge*, int>::iterator it;
					it = edges_table.find(&r_h_edge_list[invalid_macros[i] + 1][j]);
					if(it != edges_table.end()){
						it->second ++;
					}
					else{
						edges_table.insert(make_pair(&r_h_edge_list[invalid_macros[i] + 1][j], 1));
					}
				}
			}
		}
		// Vertical_edge : invalid -> neighbor
		for(int j = 0; j < v_edge_list[invalid_macros[i] + 1].size(); j++){
			// .to can't be sink
			if(v_edge_list[invalid_macros[i] + 1][j].to != macro.size() + 1){
				// top neighbor overlaps with region created by invalid_macro
				if(macro[v_edge_list[invalid_macros[i] + 1][j].to - 1]->y1() < top){
					// If the edge hasn't been found, add it into global table
					map<edge*, int>::iterator it;
					it = edges_table.find(&v_edge_list[invalid_macros[i] + 1][j]);
					if(it != edges_table.end()){
						it->second ++;
					}
					else{
						edges_table.insert(make_pair(&v_edge_list[invalid_macros[i] + 1][j], 1));
					}
				}
			}
		}		
		// Vertical_edge : neighbor -> invalid
		for(int j = 0; j < r_v_edge_list[invalid_macros[i] + 1].size(); j++){
			// .from can't be source
			if(r_v_edge_list[invalid_macros[i] + 1][j].from != 0){
				// bottom neighbor overlaps with region created by invalid_macro
				if(macro[r_v_edge_list[invalid_macros[i] + 1][j].from - 1]->y2() > bottom){
					// If the edge hasn't been found, add it into global table					
					map<edge*, int>::iterator it;
					it = edges_table.find(&r_v_edge_list[invalid_macros[i] + 1][j]);
					if(it != edges_table.end()){
						it->second ++;
					}
					else{
						edges_table.insert(make_pair(&r_v_edge_list[invalid_macros[i] + 1][j], 1));
					}
				}
			}
		}
	}
	// For each invalid_macro, we change edge's weight which has largest count
	for(int i = 0; i < invalid_macros.size(); i++){
		edge* h_target = NULL, *v_target = NULL;
		int h_max_count = 0, v_max_count = 0;
		map<edge*, int>::iterator it;
		for(int j = 0; j < h_edge_list[invalid_macros[i] + 1].size(); j++){
			// Search invalid macro's right edge. If it has maximum count, updates target edge.
			it = edges_table.find(&h_edge_list[invalid_macros[i] + 1][j]);
			if(it != edges_table.end()){
				if(it->second > h_max_count){
					h_target = &h_edge_list[invalid_macros[i] + 1][j];
					h_max_count = it->second;
				}
			}
		}
		for(int j = 0; j < v_edge_list[invalid_macros[i] + 1].size(); j++){
			// Search invalid macro's up edge. If it has maximum count, updates target edge.
			it = edges_table.find(&v_edge_list[invalid_macros[i] + 1][j]);
			if(it != edges_table.end()){
				if(it->second > v_max_count){
					v_target = &v_edge_list[invalid_macros[i] + 1][j];
					v_max_count = it->second;
				}
			}
		}
		for(int j = 0; j < r_h_edge_list[invalid_macros[i] + 1].size(); j++){
			// Search invalid macro's left edge. If it has maximum count, updates target edge.
			it = edges_table.find(&r_h_edge_list[invalid_macros[i] + 1][j]);
			if(it != edges_table.end()){
				if(it->second > h_max_count){
					h_target = &r_h_edge_list[invalid_macros[i] + 1][j];
					h_max_count = it->second;
				}
			}
		}
		for(int j = 0; j < r_v_edge_list[invalid_macros[i] + 1].size(); j++){
			// Search invalid macro's down edge. If it has maximum count, updates target edge.
			it = edges_table.find(&r_v_edge_list[invalid_macros[i] + 1][j]);
			if(it != edges_table.end()){
				if(it->second > v_max_count){
					v_target = &r_v_edge_list[invalid_macros[i] + 1][j];
					v_max_count = it->second;
				}
			}
		}
		// Horizontal target edge has higher count than vertical edge.
		if(h_max_count > v_max_count){
			for(int j = 0; j < h_edge_list[h_target->from].size(); j++){
				if(h_edge_list[h_target->from][j].to == h_target->to){
					h_edge_list[h_target->from][j].weight = (macro[h_target->from - 1]->w() + macro[h_target->to - 1]->w()) / 2 + powerplan_width;
				}
			}
			for(int j = 0; j < r_h_edge_list[h_target->to].size(); j++){
				if(r_h_edge_list[h_target->to][j].from == h_target->from){
					r_h_edge_list[h_target->to][j].weight = (macro[h_target->from - 1]->w() + macro[h_target->to - 1]->w()) / 2 + powerplan_width;
				}
			}
		}
		// Vertical target edge has higher count than horizontal edge.
		else{
			for(int j = 0; j < v_edge_list[v_target->from].size(); j++){
				if(v_edge_list[v_target->from][j].to == v_target->to){
					v_edge_list[v_target->from][j].weight = (macro[v_target->from - 1]->h() + macro[v_target->to - 1]->h()) / 2 + powerplan_width;
				}
			}
			for(int j = 0; j < r_v_edge_list[v_target->to].size(); j++){
				if(r_v_edge_list[v_target->to][j].from == v_target->from){
					r_v_edge_list[v_target->to][j].weight = (macro[v_target->from - 1]->h() + macro[v_target->to - 1]->h()) / 2 + powerplan_width;
				}
			}			
		}
	}
}