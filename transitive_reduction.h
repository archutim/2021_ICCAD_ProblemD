#include <vector>
#include "graph.h"
#include "macro.h"

using namespace std;

// O(dθ|E|), θ: average cost of answering a reachability query, d = degree.
void transitive_reduction(Graph& G, vector<Macro*>& macro);