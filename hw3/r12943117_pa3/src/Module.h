//////////////////  WARNING /////////////////////////
// DO NOT MODIFY THIS FILE, THIS IS THE HEADER FILE
// FOR THE PRECOMPILED LIBRARY.
// IF YOU WANT TO MODIFY THIS FILE, PLEASE ENSURE 
// YOU UNDERSTAND WHAT YOU ARE DOING.
/////////////////////////////////////////////////////

#ifndef MODULE_H
#define MODULE_H

#include <string>
#include <vector>
#include <iostream>
using namespace std;

#include "Pin.h"
#include "Rectangle.h"


class Module
{
public:
    enum Orient {OR_N, OR_W, OR_S, OR_E, OR_FN, OR_FW, OR_FS, OR_FE};

    Module(string name = "" , double width = -1, double height = -1, bool isFixed = false)
        : _name(name), _x(-1), _y(-1), _width(width), _height(height), _isFixed(isFixed), _orient(OR_N)
    {}

    /////////////////////////////////////////////
    // get
    /////////////////////////////////////////////
    string name() {return _name;}
    double x() {return _x;} //x coordinates
    double y() {return _y;} //y coordinates //(x,y): lower-left point of the block
    double width() {return _width;}
    double height() {return _height;}
    bool isFixed() {return _isFixed;} //if fixed module, return true

    double centerX() {return _x + _width/2;}
    double centerY() {return _y + _height/2;}
    double area() {return _width * _height;}

    Rectangle rectangle() { return Rectangle(_x, _y, _x+_width, _y+_height);}//module rectangle

    Orient orient() {return _orient;}
    string orientString() {
        const char *orientString[] = {"N", "W", "S", "E", "FN", "FW", "FS", "FE"};
        return orientString[_orient];
    }

    /////////////////////////////////////////////
    // set
    /////////////////////////////////////////////
    void setName(const string &name) {
        _name = name;
    }
    void setPosition(double x, double y) { //would update the pin positions when you set new position
        _x = x;
        _y = y;
        updatePinPositions(); //update pin positions
    }
    void setCenterPosition(double x, double y) {
        _x = x - _width/2;
        _y = y - _height/2;
        updatePinPositions();
    }
    void setWidth(double width) {
        _width = width;
    }
    void setHeight(double height) {
        _height = height;
    }
    void setIsFixed(bool isFixed) {
        _isFixed = isFixed;
    }
    void setOrient(Orient orient) {
        // swap width and height
        if ((_orient % 2) != (orient % 2)) {
            swap(_width, _height);
        }

        // flip back
        if (_orient >= 4) {
            for (unsigned i = 0; i < numPins(); i++) {
                Pin &pin = this->pin(i);
                pin.setOffset(-pin.xOffset(), pin.yOffset());
            }
        }

        // rotate 90 degree (counter-clockwise)
        int rotateDistance = (8 + orient - _orient) % 4;
        for (unsigned i = 0; i < numPins(); i++) {
            Pin &pin = this->pin(i);
            for (int k = 0; k < rotateDistance; k++) {
                double xOffset = pin.xOffset();
                double yOffset = pin.yOffset();
                pin.setOffset(-yOffset, xOffset);
            }
        }

        // flip if needed
        if (orient >= 4) {
            for (unsigned i = 0; i < numPins(); i++) {
                Pin &pin = this->pin(i);
                pin.setOffset(-pin.xOffset(), pin.yOffset());
            }
        }

        updatePinPositions();

        // change orient
        _orient = orient;
    }

    /////////////////////////////////////////////
    // get (for pins of this modules)
    /////////////////////////////////////////////
    unsigned numPins() {return _pPins.size();}
    Pin& pin(unsigned index) {return *_pPins[index];}

    /////////////////////////////////////////////
    // set (for pins of this modules)
    /////////////////////////////////////////////
    void setNumPins(unsigned numPins) {_pPins.resize(numPins);}
    void addPin(Pin *pPin) {_pPins.push_back(pPin);}
    void clearPins() {_pPins.clear();}

private:
    // variables from benchmark input
    string _name;
    double _x, _y; // low x and low y
    double _width, _height;
    bool _isFixed;
    Orient _orient;

    // pins of the module
    vector<Pin*> _pPins;

    // update pin positions
    void updatePinPositions() {
        for (unsigned i = 0; i < numPins(); i++) {
            Pin &pin = this->pin(i);
            pin.setPosition(centerX()+pin.xOffset(), centerY()+pin.yOffset());
        }
    }
};

class Node
{
    friend class Cell;

public:
    // Constructor and destructor
    Node(const int& id) :
        _id(id), _prev(NULL), _next(NULL) { }
    ~Node() { }

    // Basic access methods
    int getId() const       { return _id; }
    Node* getPrev() const   { return _prev; }
    Node* getNext() const   { return _next; }

    // Set functions
    void setId(const int& id) { _id = id; }
    void setPrev(Node* prev)  { _prev = prev; }
    void setNext(Node* next)  { _next = next; }

private:
    int         _id;    // id of the node (indicating the cell)
    Node*       _prev;  // pointer to the previous node
    Node*       _next;  // pointer to the next node
};

class Cell
{
public:
    // Constructor and destructor
    Cell(bool part, int id, int cell_id) :
        _gain(0), _pinNum(0), _part(part), _lock(false), _cellId(cell_id) {
        _node = new Node(id);
    }
    ~Cell() { 
        delete _node;
    }

    int             _sort_parameter;      // gain of the cell
    int             _cellId;
 
    // Basic access methods
    int getGain() const     { return _gain; }
    int getPinNum() const   { return _pinNum; }
    bool getPart() const    { return _part; }
    bool getLock() const    { return _lock; }
    Node* getNode() const   { return _node; }
    int getFirstNet() const { return _netList[0]; }
    vector<int> getNetList() const  { return _netList; }

    // Set functions
    void setNode(Node* node)        { _node = node; }
    void setGain(const int gain)    { _gain = gain; }
    void setPart(const bool part)   { _part = part; }

    // Modify methods
    void move()         { _part = !_part; }
    void lock()         { _lock = true; }
    void unlock()       { _lock = false; }
    void incGain()      { ++_gain; }
    void decGain()      { --_gain; }
    void incPinNum()    { ++_pinNum; }
    void decPinNum()    { --_pinNum; }
    void addNet(const int netId) { _netList.push_back(netId); }

private:
    int             _gain;      // gain of the cell
    int             _pinNum;    // number of pins the cell are connected to
    bool            _part;      // partition the cell belongs to (0-A, 1-B)
    bool            _lock;      // whether the cell is locked
    Node*           _node;      // node used to link the cells together
    vector<int>     _netList;   // list of nets the cell is connected to
};

class Result{
public:
    vector<int> cell1, cell2;
    vector<vector<int>> net1, net2;
};

#endif // MODULE_H
