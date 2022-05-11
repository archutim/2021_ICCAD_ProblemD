#include "graph.h"
#include "macro.h"
#include "flow.h"
#include <vector>

// O(y(V + E)), y = |Gc|.
bool build_Gc(vector<edge>& critical_edges, Graph &G, DINIC &Gc, vector<Macro*>& macros, Graph &G_the_other_dir, bool test_g_is_horizontal);

void cut_and_add(Graph &G, Graph &G_the_other_dir, DINIC &Gc, vector<Macro*>& macros, bool is_G_horizontal);

// O(ky(V + E)), y = |Gc|, k = adjustment times.
bool adjustment(Graph &Gh, Graph &Gv, vector<Macro*>& macros, bool init);

void Ldfs(vector<edge>& critical_edges, vector<edge>* edge_list, vector<Macro*>& macros, double* L, bool* visited, int u, bool is_horizontal);

void Rdfs(vector<edge>& critical_edges, vector<edge>* edge_list, vector<Macro*>& macros, double* R, bool* visited, int u, bool is_horizontal);

void compression(Graph& Gh, Graph& Gv, vector<Macro*>& macros);