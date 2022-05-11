#include "corner_stitch/utils/update.h"

void improve_strategy1(vector<Macro*>& macro, vector<Macro*>& native_macro, Plane* horizontal_plane, Graph& Gh, Graph& Gv);
void improve_strategy2(vector<Macro*>& macro, Plane* horizontal_plane, Plane* vertical_plane, Graph& Gh, Graph& Gv);
void improve_strategy3(vector<Macro*>& macro, Plane* horizontal_plane, Plane* vertical_plane);
void improve_strategy4(Graph& Gh, Graph& Gv, vector<Macro*>& macro, const vector<Macro*>& native_macro);
void improve_strategy5(vector<Macro*>& macro, Plane* horizontal_plane, Plane* vertical_plane, Graph& Gh, Graph& Gv);