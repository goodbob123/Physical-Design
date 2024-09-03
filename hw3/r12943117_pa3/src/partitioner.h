#ifndef PARTITIONER_H
#define PARTITIONER_H

#include <fstream>
#include <vector>
#include <map>
#include "Module.h"
#include "Net.h"
using namespace std;

class Partitioner
{
public:
    // constructor and destructor
    Partitioner(vector<Cell*> modules, vector<Net_partition*> nets, double alpha) :
        _cutSize(0), _maxPinNum(0), _netNum(nets.size()), _cellNum(modules.size()), _bFactor(alpha),
        _accGain(0), _maxAccGain(0), _iterNum(0) {
        _netArray = nets;
        _cellArray = modules;
        _partSize[0] = 0;
        _partSize[1] = 0;
    }
    ~Partitioner() {
        clear();
    }

    // basic access methods
    int getCutSize() const          { return _cutSize; }
    int getNetNum() const           { return _netNum; }
    int getCellNum() const          { return _cellNum; }
    double getBFactor() const       { return _bFactor; }
    int getPartSize(int part) const { return _partSize[part]; }

    // modify method
    Result partition();
    void update_gain(Cell*, bool);
    void update_bucket(Cell*, int);
    void calculate_cut();

    // member functions about reporting
    void printSummary() const;
    void reportNet() const;
    void reportCell() const;
    void writeResult(fstream& outFile);

private:
    int                 _cutSize;       // cut size
    int                 _partSize[2];   // size (cell number) of partition A(0) and B(1)
    int                 _netNum;        // number of nets
    int                 _cellNum;       // number of cells
    int                 _maxPinNum;     // Pmax for building bucket list
    double              _bFactor;       // the balance factor to be met
    Node*               _maxGainCell;   // pointer to max gain cell
    vector<Net_partition*>        _netArray;      // net array of the circuit
    vector<Cell*>       _cellArray;     // cell array of the circuit
    map<int, Node*, greater<int>>     _bList[2];      // bucket list of partition A(0) and B(1)

    vector<int>         _accGain;       // accumulative gain
    vector<Cell*>       _accCell;       // accumulative cell
    int                 _AccGain;       // accumulative gain
    int                 _maxAccGain;    // maximum accumulative gain
    int                 _moveNum;       // number of cell movements
    int                 _iterNum;       // number of iterations
    int                 _bestMoveNum;   // store best number of movements
    int                 _unlockNum[2];  // number of unlocked cells
    vector<int>         _moveStack;     // history of cell movement

    // Clean up partitioner
    void clear();
};

#endif  // PARTITIONER_H
