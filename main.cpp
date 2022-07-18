#include "handler.h"
#include "flow.h"
#include "graph.h"
#include "io.h"
#include "macro.h"
#include "LP.h"
#include "strategies.h"
#include "evaluation.h"
#include "constraint_graph.h"
#include "adjustment.h"
#include "compression.h"
#include "transitive_reduction.h"
#include "corner_stitch/utils/update.h"
#include <iostream>
#include <random>
#include <vector>
#include <cmath>
#include <unistd.h>
#include <signal.h>
#include <chrono>
using namespace std;

// Iuput Information
double chip_width;
double chip_height;
int micron;
int macros_num;
double alpha;
double beta;
double buffer_constraint;
double powerplan_width;
double min_spacing;

// handler use do_SA to stop SA loop when running time close to limit
bool do_SA = true;

// globally acceesible macros vector
vector<Macro *> og_macros;

double displacement_evaluation(vector<Macro*>& macro, vector<Macro*>& native_macro){
	double displacement_cost = 0;
	for(int i = 0; i < native_macro.size(); i++){
		displacement_cost += abs(macro[i]->x1() - native_macro[i]->x1());
		displacement_cost += abs(macro[i]->y1() - native_macro[i]->y1());
	}
	return displacement_cost / micron;
}

double total_cost(double displace, double powerplan){
	return alpha * displace + beta * sqrt(powerplan);
}

vector<Macro*> macros_best;
IoData* IO_data_ptr = NULL;
ofstream myfile;
int main(int argc, char *argv[])
{
	// Assign event handlers(timeout, ctrl^C)
	signal(SIGINT, sigint_handler);
	signal(SIGALRM, sigalrm_handler);  // set a signal handler
	alarm(890);  // set an alarm for 15*60 seconds from now

	// Parse Input files, O(n).
	IoData IO_data(argc, argv);

	// Store design info and parameter
	IO_data_ptr = &IO_data;
	chip_width = (double)IO_data.die_width;	
	chip_height = (double)IO_data.die_height;
	micron = IO_data.dbu_per_micron;
	macros_num = IO_data.macros.size();
	alpha = (double)IO_data.weight_alpha;
	beta = (double)IO_data.weight_beta;
	buffer_constraint = (double)IO_data.buffer_constraint;
	powerplan_width = (double)IO_data.powerplan_width_constraint;
	min_spacing = (double)IO_data.minimum_spacing;

	// Store original position for evaluation
	vector<Macro*> native_macros;
	for(int i = 0; i < IO_data.macros.size(); i++){
		native_macros.push_back(new Macro(IO_data.macros[i]->w(), IO_data.macros[i]->h(),
											IO_data.macros[i]->x1(), IO_data.macros[i]->y1(),
											IO_data.macros[i]->is_fixed(), IO_data.macros[i]->id()));
	}


	og_macros = IO_data.macros;
	Graph Gh(macros_num), Gv(macros_num);
	build_init_constraint_graph(Gh, Gv, og_macros);
	transitive_reduction(Gh, og_macros);
	transitive_reduction(Gv, og_macros);
	adjustment(Gh, Gv, og_macros, true);
	// compression(Gh, Gv, og_macros);
	// adjustment(Gh, Gv, og_macros, true);
	Linear_Program(og_macros, Gv, Gh);

	// Create horizontal, vertical corner stitch data structure
	Plane* horizontal_plane = CreateTilePlane(), *vertical_plane = CreateTilePlane();

	// Calculate powerplan cost
	double displacement, powerplan_cost;
	displacement = displacement_evaluation(og_macros, native_macros);
	powerplan_cost = cost_evaluation(og_macros, horizontal_plane, vertical_plane);

	// Vector stores invalid macros found by corner stitch data structure
	vector<int> invalid_macros;
	invalid_macros = invalid_check(og_macros, horizontal_plane);

	// Should add invalid penalty into cost
	double init_cost = total_cost(displacement, powerplan_cost);
	printf("Initial cost = %lf(%lf, %lf)\n", init_cost, displacement, powerplan_cost);
	double cost_now = init_cost;

	// "best" is for storing best solution, "next" presents perturbed solution info.
	double cost_best, cost_next, best_disp, best_area;
	vector<Macro *> macros_next(macros_num);
	vector<Macro *> macros_checkpoint(macros_num);
	vector<int> checkpoint_invalid = invalid_macros;
	macros_best.resize(macros_num);
	Graph Gv_next(macros_num), Gh_next(macros_num), Gv_best(macros_num), Gh_best(macros_num), Gh_checkpoint(macros_num), Gv_checkpoint(macros_num);
	
	// ===========================TO DO===========================
	// if initial solution is invalid, do not accept this solution
	// ===========================================================
	Gv_best.Copy(Gv);
	Gh_best.Copy(Gh);
	Gh_checkpoint.rebuild();
	Gv_checkpoint.rebuild();	
	Gh_checkpoint.Copy(Gh);
	Gv_checkpoint.Copy(Gv);
	for(int j = 0; j < macros_num; j++){
		macros_best[j] = new Macro(*og_macros[j]);
		macros_checkpoint[j] = new Macro(*og_macros[j]);
	}
	if(invalid_macros.empty()){
		cost_best = cost_now;
		best_disp = displacement;
		best_area = powerplan_cost;
	}
	else{
		cost_now = DBL_MAX;
		cost_best = DBL_MAX;
		best_disp = DBL_MAX;
		best_area = DBL_MAX;
	}
	// Set random seed for chosing improvement stratigy.
	srand(2598219);
	double* cost_history = new double[9000];
	double* appr_history = new double[9000];
	int count = 0;
	auto timestamp = std::chrono::system_clock::now();
	time_t start_time = time(NULL);
	while(do_SA){
		std::chrono::duration<double> diff = std::chrono::system_clock::now() - timestamp;
		if(diff.count() > 0.1){
			cost_history[count] = cost_now;
			count++;
			timestamp = std::chrono::system_clock::now();
		}
		if(time(NULL) - start_time >= 5){
			double x_sum = 0, y_sum = 0, x2_sum = 0, xy_sum = 0;
			if(cost_history[0] == DBL_MAX)
				cost_history[0] = cost_history[1];
			for(int i = 0; i < count; i++){
				appr_history[i] = cost_history[i] - cost_now + 1;
				x_sum += (i + 1);
				x2_sum += (i + 1) * (i + 1);
				y_sum += log(appr_history[i]);
				xy_sum += (i + 1) * log(appr_history[i]);
			}
			double B = (count*xy_sum-x_sum*y_sum)/(count*x2_sum-x_sum*x_sum),
					A = (y_sum - B * x_sum) / count;
			double a = pow(2.71828, A), b = pow(2.71828, B);
			if(a * log(b) * pow(b, count) > -0.01){
				Gv.rebuild();
				Gh.rebuild();
				Gv.Copy(Gv_checkpoint);
				Gh.Copy(Gh_checkpoint);
				cost_now = init_cost;
				for(int k=0;k<macros_num;k++){
					delete og_macros[k];
					og_macros[k] = new Macro(*macros_checkpoint[k]);
				}
				invalid_macros.clear();
				invalid_macros = checkpoint_invalid;
				count = 0;
				cost_history[count] = cost_now;
			}
			start_time = time(NULL);
		}
		Gv_next.rebuild();
		Gh_next.rebuild();
		Gv_next.Copy(Gv);
		Gh_next.Copy(Gh);
		for(int j = 0; j < macros_num; j++){
			delete macros_next[j];
			macros_next[j] = new Macro(*og_macros[j]);
		}

		//   perturb
		bool legalization = false;
		if(!invalid_macros.empty()){
			fix_invalid(macros_next, invalid_macros, Gh_next, Gv_next);
			adjustment(Gh_next, Gv_next, macros_next, true);
			legalization = true;
		}
		// If current placement result is legal, then we try to improve our result
		else{
			// improvement strategy :
			// 1. force macros near boundary to align boundary
			// 2. find out pairs of two macros cause cost, seperate them by manipulating macros directly or changing edge weight.
			// 3. move macros by checking if move the macro can get cost reduction detailly.
			// 4. if the macro has displacement more than mean value by a standard deviation, try to move it back to origin position. 
			int op = rand() % 31;
			if(op < 1){
				improve_strategy1(macros_next, native_macros, horizontal_plane, Gh_next, Gv_next);
			}
			else if(op < 11){
				improve_strategy4(Gh_next, Gv_next, macros_next, native_macros);
				transitive_reduction(Gh_next, macros_next);
				transitive_reduction(Gv_next, macros_next);
				if(!adjustment(Gh_next, Gv_next, macros_next, false)){
					continue;
				}			
			}
			else if(op < 21){
				improve_strategy2(macros_next, horizontal_plane, vertical_plane, Gh_next, Gv_next);
				if(!adjustment(Gh_next, Gv_next, macros_next, false)){
					continue;
				}
			}
			else{
				improve_strategy5(macros_next, horizontal_plane, vertical_plane, Gh_next, Gv_next);
				if(!adjustment(Gh_next, Gv_next, macros_next, false)){
					continue;
				}
			}
		}
		
		Linear_Program(macros_next, Gv_next, Gh_next);

		RemoveTilePlane(horizontal_plane);
		RemoveTilePlane(vertical_plane);

		horizontal_plane = CreateTilePlane();
		vertical_plane = CreateTilePlane();

		displacement = displacement_evaluation(macros_next, native_macros);
		powerplan_cost = cost_evaluation(macros_next, horizontal_plane, vertical_plane);
		cost_next = total_cost(displacement, powerplan_cost);

		invalid_macros.clear();
		invalid_macros = invalid_check(macros_next, horizontal_plane);
		// ===========================TO DO========================
		// macro_next can accept solution contains invalid macros
		// macro_best can't accept solution contains invalid macros
		// ========================================================
		cout<<"SA before copy, cost(next, now):"<<cost_next<<", "<<cost_now<<endl;
		if(cost_next < cost_best && invalid_macros.empty()){
			Gv_best.rebuild();
			Gh_best.rebuild();
			Gv_best.Copy(Gv_next);
			Gh_best.Copy(Gh_next);
			cost_best = cost_next;
			best_disp = displacement;
			best_area = powerplan_cost;
			// myfile << count << "," << cost_best << "\n";
			// count++;
			for(int k=0;k<macros_num;k++){
				delete macros_best[k];
				macros_best[k] = new Macro(*macros_next[k]);
			}
		}
		if((cost_next < cost_now && invalid_macros.empty()) || legalization){
			Gv.rebuild();
			Gh.rebuild();
			Gv.Copy(Gv_next);
			Gh.Copy(Gh_next);
			cost_now = cost_next;
			for(int k=0;k<macros_num;k++){
				delete og_macros[k];
				og_macros[k] = new Macro(*macros_next[k]);
			}
		}
		else{
			// if macro_next has invalid macros and macro_next is rejected, we need to clear the vector
			invalid_macros.clear();
		}
		cout<<"SA after copy, costNow:"<<cost_now<<", costBest:"<<cost_best << "(" << best_disp << ", " << best_area << ")" <<endl;
	}

	IO_data.macros = macros_best;
	IO_data.output();
	return 0;
}
