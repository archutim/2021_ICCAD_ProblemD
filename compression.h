#include "graph.h"
#include "macro.h"
#include "flow.h"
#include <vector>

void compression_build_Gc(vector<edge>& critical_edges, Graph &G, Graph &G_the_other_dir, DINIC &Gc, vector<Macro*>& macros, double target_value, bool is_G_horizontal);

void Ldfs(vector<edge>& critical_edges, vector<edge>* edge_list, vector<Macro*>& macros, double* L, bool* visited, int u, bool is_horizontal);

void Rdfs(vector<edge>& critical_edges, vector<edge>* edge_list, vector<Macro*>& macros, double* R, bool* visited, int u, bool is_horizontal);

void compression(Graph& Gh, Graph& Gv, vector<Macro*>& macros);