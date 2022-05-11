#include <iostream>
#include <vector>
#include "macro.h"
#include "graph.h"

bool is_overlapped(Macro &m1, Macro &m2);

bool x_dir_is_overlapped_less(Macro &m1, Macro &m2);

bool x_dir_projection_no_overlapped(Macro &m1, Macro &m2);

bool y_dir_projection_no_overlapped(Macro &m1, Macro &m2);

bool projection_no_overlapped(Macro &m1, Macro &m2);

double determine_edge_weight(Macro *m1, Macro *m2, bool is_horizontal);

// O(1)
void add_st_nodes(Graph &Gh, Graph &Gv, Macro* macros);

// O(n)
void add_macro_to_graph(Graph &Gh, Graph &Gv, vector<Macro *> macros, int adding_macro, bool build);

// O(n^2)
void build_init_constraint_graph(Graph &Gh, Graph &Gv, vector<Macro *> macros); // O(n^2)