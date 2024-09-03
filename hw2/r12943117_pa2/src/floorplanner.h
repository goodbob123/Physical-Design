#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include "module.h"
using namespace std;

class ContourNode {
public:
    ContourNode(int a, int b, ContourNode* c) :
        x(a), y(b), next(c) {}

    int x;
    int y;
    ContourNode* next;
};

class Floorplanner
{
public:
    //random


    // constructor and destructor
    Floorplanner(fstream& input_blk, fstream& input_net) {
        net_list.clear();
        read_block(input_blk);
        read_net(input_net);
    }
    ~Floorplanner()  { 
    }

    void floorplan(fstream& output);

    // handle input functions
    void read_block(fstream& input_blk);
    void read_net(fstream& input_net);

    // calculate cost
    double print_HPWL();
    double HPWL();
    int Box_Area();
    double Cost(double alpha);

    // packing
    void packing(BStarTree tree);
    void placeBlock(Node* node, int x);



protected:
    //global
    double  max_x;                     // maximum x coordinate for all blocks
    double  max_y;                     // maximum y coordinate for all blocks

    //bounding box
    int box_width;                      // width of the bounding box
    int box_height;                     // height of the bounding box

    //block
    int num_block;
    vector<Block> block_list;

    //terminal
    int num_terminal;
    vector<Terminal> terminal_list;

    //net
    int num_net;
    vector<Net> net_list;

    //variable to variable
    unordered_map<string, Terminal*> name2terminal;
    unordered_map<string, Block*> name2block;

    //tree
    BStarTree old_tree;
    BStarTree tree;
    BStarTree best;

    //packing
    ContourNode* head;

    int A_norm;
    int W_norm;
};
