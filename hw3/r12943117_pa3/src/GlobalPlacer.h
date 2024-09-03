#define _GLIBCXX_USE_CXX11_ABI 0  // Align the ABI version to avoid compatibility issues with `Placment.h`
#ifndef GLOBALPLACER_H
#define GLOBALPLACER_H

#include "Placement.h"
#include <cstdlib>
#include <cstdio>
#include <fstream>

class GlobalPlacer 
{
public:
    GlobalPlacer(Placement &placement);
	void place(int slot_module = 160, double alpha = 0.2, int seed = 0);
    void placecell(vector<vector<double>> layout, vector<Cell*> modules);
    void plotPlacementResult( const string outfilename, bool isPrompt = false );
    void min_cut_place(vector<vector<double>> layout, vector<Cell*> modules, vector<Net_partition*> nets);
private:
    Placement& _placement;
    void plotBoxPLT( ofstream& stream, double x1, double y1, double x2, double y2 );
    double _alpha;
    int _slot_module, _seed;
};

#endif // GLOBALPLACER_H
