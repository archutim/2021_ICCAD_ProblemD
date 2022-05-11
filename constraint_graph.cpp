#include <iostream>
#include <vector>
#include "macro.h"
#include "graph.h"
#include "constraint_graph.h"

extern double min_spacing, powerplan_width;
extern int macros_num;

bool is_overlapped(Macro &m1, Macro &m2){
	return !(m1.x2() <= m2.x1() || m1.x1() >= m2.x2() ||
				m1.y2() <= m2.y1() || m1.y1() >= m2.y2());
}

bool x_dir_is_overlapped_less(Macro &m1, Macro &m2){
	double diff_x, diff_y;
	if (m1.cx() > m2.cx())
		diff_x = m2.x2() - m1.x1();
	else
		diff_x = m1.x2() - m2.x1();
	if (m1.cy() < m2.cy())
		diff_y = m1.y2() - m2.y1();
	else
		diff_y = m2.y2() - m1.y1();
	return diff_x < diff_y;
}

bool x_dir_projection_no_overlapped(Macro &m1, Macro &m2){
	if (m1.cx() > m2.cx())
		return m1.x1() >= m2.x2();
	else
		return m2.x1() >= m1.x2();
}

bool y_dir_projection_no_overlapped(Macro &m1, Macro &m2){
	if (m1.cy() > m2.cy())
		return m1.y1() >= m2.y2();
	else
		return m2.y1() >= m1.y2();
}

bool projection_no_overlapped(Macro &m1, Macro &m2){
	return x_dir_projection_no_overlapped(m1, m2) && y_dir_projection_no_overlapped(m1, m2);
}

double determine_edge_weight(Macro *m1, Macro *m2, bool is_horizontal){
	double w = (is_horizontal) ? (m1->w() + m2->w()) / 2 : (m1->h() + m2->h()) / 2;
	if (m1->name() != "null" && m2->name() != "null"){
		w += min_spacing;
	}
	return w;
}

void add_st_nodes(Graph &Gh, Graph &Gv, Macro* macro)
{
	// id 0 for source, V+1 for sink
	if (macro->is_fixed())
	{
		Gh.add_edge(0, macro->id(), macro->cx() - 0);
		Gh.add_edge(macro->id(), macros_num + 1, chip_width - macro->cx());
		Gv.add_edge(0, macro->id(), macro->cy() - 0);
		Gv.add_edge(macro->id(), macros_num + 1, chip_height - macro->cy());
	}
	else
	{
		Gh.add_edge(0, macro->id(), macro->w() / 2);
		Gh.add_edge(macro->id(), macros_num + 1, macro->w() / 2);
		Gv.add_edge(0, macro->id(), macro->h() / 2);
		Gv.add_edge(macro->id(), macros_num + 1, macro->h() / 2);
	}
}

void add_macro_to_graph(Graph &Gh, Graph &Gv, vector<Macro *> macros, int adding_macro, bool build){
	int define_i = adding_macro, define_j = 0;
	if(build){
		define_j = adding_macro + 1;
	}
	for (; define_j < macros_num; define_j++){
		if(define_i == define_j){
			continue;
		}
		int i = 0, j = 0;
		if(define_i < define_j){
			i = define_i;
			j = define_j;
		}
		else{
			i = define_j;
			j = define_i;
		}
		double h_weight = determine_edge_weight(macros[i], macros[j], true);
		double v_weight = determine_edge_weight(macros[i], macros[j], false);
		bool i_is_at_the_bottom = false, i_is_at_the_left = false;
		if (macros[i]->cx() < macros[j]->cx())
			i_is_at_the_left = true;
		if (macros[i]->cy() < macros[j]->cy())
			i_is_at_the_bottom = true;
		
		bool skip = false;
		if(macros[i]->name()=="null" && macros[j]->name()!="null"){
			if(macros[i]->x1()==0 && !i_is_at_the_left){
				if(macros[i]->y1()==0 && !i_is_at_the_bottom){
					(macros[i]->x2()-macros[j]->x1() < macros[i]->y2()-macros[j]->y1()) ?
						Gh.add_edge(macros[i]->id(), macros[j]->id(), h_weight) :
						Gv.add_edge(macros[i]->id(), macros[j]->id(), v_weight);
					skip = true;
				}else if(macros[i]->y2()==chip_height && i_is_at_the_bottom){
					(macros[i]->x2()-macros[j]->x1() < macros[j]->y2()-macros[i]->y1()) ?
						Gh.add_edge(macros[i]->id(), macros[j]->id(), h_weight) :
						Gv.add_edge(macros[j]->id(), macros[i]->id(), v_weight);
					skip = true;
				}
			}else if(macros[i]->x2()==chip_width && i_is_at_the_left){
				if(macros[i]->y1()==0 && !i_is_at_the_bottom){
					(macros[j]->x2()-macros[i]->x1() < macros[i]->y2()-macros[j]->y1()) ?
						Gh.add_edge(macros[j]->id(), macros[i]->id(), h_weight) :
						Gv.add_edge(macros[i]->id(), macros[j]->id(), v_weight);
					skip = true;
				}else if(macros[i]->y2()==chip_height && i_is_at_the_bottom){
					(macros[j]->x2()-macros[i]->x1() < macros[j]->y2()-macros[i]->y1()) ?
						Gh.add_edge(macros[j]->id(), macros[i]->id(), h_weight) :
						Gv.add_edge(macros[j]->id(), macros[i]->id(), v_weight);
					skip = true;
				}
			}
		}
		else if(macros[j]->name()=="null" && macros[i]->name()!="null"){
			if(macros[j]->x1()==0 && i_is_at_the_left){
				if(macros[j]->y1()==0 && i_is_at_the_bottom){
					(macros[j]->x2()-macros[i]->x1() < macros[j]->y2()-macros[i]->y1()) ?
						Gh.add_edge(macros[j]->id(), macros[i]->id(), h_weight) :
						Gv.add_edge(macros[j]->id(), macros[i]->id(), v_weight);
					skip = true;
				}else if(macros[j]->y2()==chip_height && !i_is_at_the_bottom){
					(macros[j]->x2()-macros[i]->x1() < macros[i]->y2()-macros[j]->y1()) ?
						Gh.add_edge(macros[j]->id(), macros[i]->id(), h_weight) :
						Gv.add_edge(macros[i]->id(), macros[j]->id(), v_weight);
					skip = true;
				}
			}else if(macros[j]->x2()==chip_width && !i_is_at_the_left){
				if(macros[j]->y1()==0 && i_is_at_the_bottom){
					(macros[i]->x2()-macros[j]->x1() < macros[j]->y2()-macros[i]->y1()) ?
						Gh.add_edge(macros[i]->id(), macros[j]->id(), h_weight) :
						Gv.add_edge(macros[j]->id(), macros[i]->id(), v_weight);
					skip = true;
				}else if(macros[j]->y2()==chip_height && !i_is_at_the_bottom){
					(macros[i]->x2()-macros[j]->x1() < macros[i]->y2()-macros[j]->y1()) ?
						Gh.add_edge(macros[i]->id(), macros[j]->id(), h_weight) :
						Gv.add_edge(macros[i]->id(), macros[j]->id(), v_weight);
					skip = true;
				}
			}
		}
		if(skip)
			continue;

		if (is_overlapped(*macros[i], *macros[j]))
		{
			if (x_dir_is_overlapped_less(*macros[i], *macros[j]))
			{
				(i_is_at_the_left) ? Gh.add_edge(macros[i]->id(), macros[j]->id(), h_weight)
									: Gh.add_edge(macros[j]->id(), macros[i]->id(), h_weight);
			}
			else
			{
				(i_is_at_the_bottom) ? Gv.add_edge(macros[i]->id(), macros[j]->id(), v_weight)
										: Gv.add_edge(macros[j]->id(), macros[i]->id(), v_weight);
			}
		}
		else if (projection_no_overlapped(*macros[i], *macros[j]))
		{
			double diff_x, diff_y;
			diff_x = (i_is_at_the_left) ? macros[j]->x1() - macros[i]->x2()
										: macros[i]->x1() - macros[j]->x2();
			diff_y = (i_is_at_the_bottom) ? macros[j]->y1() - macros[i]->y2()
											: macros[i]->y1() - macros[j]->y2();
			if (diff_x < diff_y)
			{
				(i_is_at_the_bottom) ? Gv.add_edge(macros[i]->id(), macros[j]->id(), v_weight)
										: Gv.add_edge(macros[j]->id(), macros[i]->id(), v_weight);
			}
			else
			{
				(i_is_at_the_left) ? Gh.add_edge(macros[i]->id(), macros[j]->id(), h_weight)
									: Gh.add_edge(macros[j]->id(), macros[i]->id(), h_weight);
			}
		}
		else
		{
			if (x_dir_projection_no_overlapped(*macros[i], *macros[j]))
			{

				(i_is_at_the_left) ? Gh.add_edge(macros[i]->id(), macros[j]->id(), h_weight)
									: Gh.add_edge(macros[j]->id(), macros[i]->id(), h_weight);
			}
			else if (y_dir_projection_no_overlapped(*macros[i], *macros[j]))
			{
				(i_is_at_the_bottom) ? Gv.add_edge(macros[i]->id(), macros[j]->id(), v_weight)
										: Gv.add_edge(macros[j]->id(), macros[i]->id(), v_weight);
			}
			else
			{
				throw "error";
			}
		}
	}
}

void build_init_constraint_graph(Graph &Gh, Graph &Gv, vector<Macro*> macros)
{
	// Since function add_macro_to_graph will access macro[adding_macro + 1],
	//	so we don't pass "macro_num - 1" as parameter.
	for(int i = 0; i < macros_num; i++){
		add_st_nodes(Gh, Gv, macros[i]);
	}
	for(int i = 0; i < macros_num - 1; i++){
		add_macro_to_graph(Gh, Gv, macros, i, true);
	}
}