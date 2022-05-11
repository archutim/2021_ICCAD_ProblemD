#include <vector>
#include "macro.h"
#include "graph.h"
#include "corner_stitch/utils/update.h"

using namespace std;

double cost_evaluation(vector<Macro*>& macro, Plane* horizontal_plane, Plane* vertical_plane);
vector<int> invalid_check(vector<Macro*>& macro, Plane* horizontal_plane);
void fix_invalid(vector<Macro*>& macro, vector<int>& invalid_macros, Graph& Gh, Graph& Gv);