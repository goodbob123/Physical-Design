#include "GlobalPlacer.h"

#include <cstdio>
#include <vector>
#include <random>
#include<unordered_map>

#include "Net.h"
#include "Module.h"
#include "partitioner.h"
#include "ObjectiveFunction.h"
#include "Optimizer.h"
#include "Point.h"

GlobalPlacer::GlobalPlacer(Placement &placement)
    : _placement(placement) {
}

void GlobalPlacer::placecell(vector<vector<double>> layout, vector<Cell*> modules){
    //std::random_device rd;
    mt19937 gen(_seed); 
    int x, y;
    for (size_t i = 0; i < modules.size(); i++) {
        uniform_real_distribution<> distri_x(layout[0][0], layout[1][0]);
        x = distri_x(gen);
        uniform_real_distribution<> distri_y(layout[0][1], layout[1][1]);
        y = distri_y(gen);
        //cout<<"place module "<<modules[i]->_cellId<<" at ("<<x<<", "<<y<<")"<<endl;
        _placement.module(modules[i]->_cellId).setPosition(x, y);
    }
}

void GlobalPlacer::place(int slot_module, double alpha, int seed) {
    // TODO: Implement your global placement algorithm here.
    _slot_module = slot_module;
    _alpha = alpha;
    _seed = seed;
    //cout<<alpha<<" "<<slot_module<<endl;
    vector<vector<double>> layout = {
        {_placement.boundryLeft() , _placement.boundryBottom()},
        {_placement.boundryRight(), _placement.boundryTop()}
    };

    vector<Net_partition*> nets;
    vector<Cell*> cells;
    for(int i = 0; i < _placement.numModules(); i++){
        cells.push_back(new Cell(0, i, i));
    }
    for(int i = 0; i < _placement.numNets(); i++){
        nets.push_back(new Net_partition());
        unordered_map<int, int> hash;
        for(int j = 0; j < _placement.net(i).numPins(); j++){
            int cellId = _placement.net(i).pin(j).moduleId();
            if(hash.find(cellId) == hash.end()){
                cells[cellId]->addNet(i);
                cells[cellId]->incPinNum();
                nets[i]->addCell(cellId);
                hash.insert(pair<int, int>(cellId, 0));
            }
        }
    }

    min_cut_place(layout, cells, nets);
}

void GlobalPlacer::min_cut_place(vector<vector<double>> layout, vector<Cell*> modules, vector<Net_partition*> nets) {
    if(modules.size() < _slot_module){
        placecell(layout, modules);
        for(int i = 0; i < modules.size(); i++){
            delete modules[i];
        }
        for(int i = 0; i < nets.size(); i++){
            delete nets[i];
        }
    }
    else{
        //partition
        Partitioner* partitioner = new Partitioner(modules, nets, _alpha);
        Result result = partitioner->partition();
        delete partitioner;

        vector<Net_partition*> nets1, nets2;
        vector<Cell*> cells1, cells2;
        for(int i = 0; i < result.cell1.size(); i++){
            cells1.push_back(new Cell(0, i, result.cell1[i]));
        }
        for(int i = 0; i < result.net1.size(); i++){
            nets1.push_back(new Net_partition());
            for(int j = 0; j < result.net1[i].size(); j++){
                int cellId = result.net1[i][j];
                cells1[cellId]->addNet(i);
                cells1[cellId]->incPinNum();
                nets1[i]->addCell(cellId);
            }
        }
        for(int i = 0; i < result.cell2.size(); i++){
            cells2.push_back(new Cell(0, i, result.cell2[i]));
        }
        for(int i = 0; i < result.net2.size(); i++){
            nets2.push_back(new Net_partition());
            for(int j = 0; j < result.net2[i].size(); j++){
                int cellId = result.net2[i][j];
                cells2[cellId]->addNet(i);
                cells2[cellId]->incPinNum();
                nets2[i]->addCell(cellId);
            }
        }

        double area1 = 0, area2 = 0;
        for(int i = 0; i < result.cell1.size(); i++){
            area1 += _placement.module(result.cell1[i]).area();
        }
        for(int i = 0; i < result.cell2.size(); i++){
            area2 += _placement.module(result.cell2[i]).area();
        }
        double ratio = area1 / (area1 + area2);
        //cut layout
        vector<vector<double>> layout1, layout2;
        if((layout[1][0] - layout[0][0]) >= (layout[1][1] - layout[1][0])){
            layout1 = {{layout[0][0], layout[0][1]}, {layout[0][0] * (1-ratio) + layout[1][0] * ratio, layout[1][1]}};
            layout2 = {{layout[0][0] * (1-ratio) + layout[1][0] * ratio, layout[0][1]}, {layout[1][0], layout[1][1]}};
        }
        else{
            layout1 = {{layout[0][0], layout[0][1]}, {layout[1][0], layout[0][1] * (1-ratio) + layout[1][1] * ratio}};
            layout2 = {{layout[0][0], layout[0][1] * (1-ratio) + layout[1][1] * ratio}, {layout[1][0], layout[1][1]}};
        }

        min_cut_place(layout1, cells1, nets1);
        min_cut_place(layout2, cells2, nets2);
    }
}

void GlobalPlacer::plotPlacementResult(const string outfilename, bool isPrompt) {
    ofstream outfile(outfilename.c_str(), ios::out);
    outfile << " " << endl;
    outfile << "set title \"wirelength = " << _placement.computeHpwl() << "\"" << endl;
    outfile << "set size ratio 1" << endl;
    outfile << "set nokey" << endl
            << endl;
    outfile << "plot[:][:] '-' w l lt 3 lw 2, '-' w l lt 1" << endl
            << endl;
    outfile << "# bounding box" << endl;
    plotBoxPLT(outfile, _placement.boundryLeft(), _placement.boundryBottom(), _placement.boundryRight(), _placement.boundryTop());
    outfile << "EOF" << endl;
    outfile << "# modules" << endl
            << "0.00, 0.00" << endl
            << endl;
    for (size_t i = 0; i < _placement.numModules(); ++i) {
        Module &module = _placement.module(i);
        plotBoxPLT(outfile, module.x(), module.y(), module.x() + module.width(), module.y() + module.height());
    }
    outfile << "EOF" << endl;
    outfile << "pause -1 'Press any key to close.'" << endl;
    outfile.close();

    if (isPrompt) {
        char cmd[200];
        sprintf(cmd, "gnuplot %s", outfilename.c_str());
        if (!system(cmd)) {
            cout << "Fail to execute: \"" << cmd << "\"." << endl;
        }
    }
}

void GlobalPlacer::plotBoxPLT(ofstream &stream, double x1, double y1, double x2, double y2) {
    stream << x1 << ", " << y1 << endl
           << x2 << ", " << y1 << endl
           << x2 << ", " << y2 << endl
           << x1 << ", " << y2 << endl
           << x1 << ", " << y1 << endl
           << endl;
}
