//////////////////  WARNING /////////////////////////
// DO NOT MODIFY THIS FILE, THIS IS THE HEADER FILE
// FOR THE PRECOMPILED LIBRARY.
// IF YOU WANT TO MODIFY THIS FILE, PLEASE ENSURE 
// YOU UNDERSTAND WHAT YOU ARE DOING.
/////////////////////////////////////////////////////

#ifndef NET_H
#define NET_H

#include <vector>
using namespace std;

#include "Pin.h"

class Net {
public:
    Net() {}

    /////////////////////////////////////////////
    // get (for pins of this net)
    /////////////////////////////////////////////
    unsigned numPins() {return _pPins.size();}
    Pin& pin(unsigned index) {return *_pPins[index];} // index: 0 ~ (numPins-1), not Pin id

    /////////////////////////////////////////////
    // set (for pins of this net)
    /////////////////////////////////////////////
    void setNumPins(unsigned numPins) {_pPins.resize(numPins);}
    void addPin(Pin *pPin) {_pPins.push_back(pPin);}
    void clearPins() {_pPins.clear();}

private:
    // pins of the module
    vector<Pin *> _pPins;
};

class Net_partition
{
public:
    // constructor and destructor
    Net_partition(){
        _partCount[0] = 0; _partCount[1] = 0;
    }
    ~Net_partition()  { }

    // basic access methods
    int getPartCount(int part) const { return _partCount[part]; }
    vector<int> getCellList()  const { return _cellList; }

    // set functions
    void setPartCount(int part, const int count) { _partCount[part] = count; }

    // modify methods
    void incPartCount(int part)     { ++_partCount[part]; }
    void decPartCount(int part)     { --_partCount[part]; }
    void addCell(const int cellId)  { _cellList.push_back(cellId); }

private:
    int             _partCount[2];  // Cell number in partition A(0) and B(1)
    vector<int>     _cellList;      // List of cells the net is connected to
    
};
#endif // NET_H
