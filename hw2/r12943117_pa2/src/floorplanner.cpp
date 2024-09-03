#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <cassert>
#include "floorplanner.h"
using namespace std;

void BStarTree::insert(Block* block){
    uniform_int_distribution<> random_Pos(0, 1);

    unique_ptr<Node> newNode = my::make_unique<Node>(block);
    list.push_back(newNode.get());
    if (!root) {
        root = move(newNode);
    } else {
        Node* current = root.get();
        while (true) {
            if (random_Pos(gen)) {  // comparison by x-coordinate
                if (!current->left) {
                    newNode->parent = current;
                    current->left = move(newNode);
                    break;
                }
                current = current->left.get();
            } else {
                if (!current->right) {
                    newNode->parent = current;
                    current->right = move(newNode);
                    break;
                }
                current = current->right.get();
            }
        }
    }
}

Node* BStarTree::insertNode(Block* block, Node* parentNode, bool insertLeft) {
    unique_ptr<Node> newNode = my::make_unique<Node>(block);
    Node *ptr = newNode.get();
    newNode->parent = parentNode;

    if (insertLeft) {
        if (parentNode->left) {
            // Move existing left child to the new node's left
            newNode->left = move(parentNode->left);
            if (newNode->left) {
                newNode->left->parent = newNode.get();
            }
        }
        parentNode->left = move(newNode);
    } else {
        if (parentNode->right) {
            // Move existing right child to the new node's right
            newNode->right = move(parentNode->right);
            if (newNode->right) {
                newNode->right->parent = newNode.get();
            }
        }
        parentNode->right = move(newNode);
    }
    return ptr;
}

int BStarTree::remove(Node* node) {
    assert(node);
    if (!node) return -1;

    Node* parent = node->parent;

    // Case: Node has two children
    if (node->left && node->right) {
        Node* successor = node->right.get();
        while (successor->left) {
            successor = successor->left.get();
        }
        node->block = successor->block;  // Swap values
        return remove(successor);  // Recursively remove the successor
    } else {
        unique_ptr<Node>& nodePtr = (node == root.get()) ? root : (node == node->parent->left.get() ? node->parent->left : node->parent->right);
        unique_ptr<Node> temp = move(nodePtr);
        
        int flag = 0;
        if (node->left) {
            flag = 1;
            nodePtr = move(node->left);
        } else if (node->right) {
            flag = 2;
            nodePtr = move(node->right);
        } else {
            nodePtr.reset();
        }

        if (nodePtr) {
            nodePtr->parent = node->parent;
        }
        return distance(list.begin(), find(list.begin(), list.end(), node));
    }
}

void BStarTree::rotate(Node* node) {
    node->block->setRotate();
}

void BStarTree::swap(Node* a, Node* b) {
    if (a == b) return;

    Node* aParent = a->parent;
    Node* bParent = b->parent;

    if (aParent) {
        if (aParent->left.get() == a) {
            aParent->left.release();
            aParent->left.reset(b);
        } else {
            aParent->right.release();
            aParent->right.reset(b);
        }
    } else {
        root.release();
        root.reset(b);
    }

    if (bParent) {
        if (bParent->left.get() == b) {
            bParent->left.release();
            bParent->left.reset(a);
        } else {
            bParent->right.release();
            bParent->right.reset(a);
        }
    } else {
        root.release();
        root.reset(a);
    }

    a->parent = bParent;
    b->parent = aParent;

    unique_ptr<Node> temp = move(a->left);
    a->left = move(b->left);
    b->left = move(temp);

    temp = move(a->right);
    a->right = move(b->right);
    b->right = move(temp);

    if (a->left) a->left->parent = a;
    if (a->right) a->right->parent = a;
    if (b->left) b->left->parent = b;
    if (b->right) b->right->parent = b;
}

void Floorplanner::read_block(fstream& input_blk){
    string buf, name;
    size_t w, h;
    input_blk>>buf>>box_width>>box_height;
    input_blk>>buf>>num_block;
    input_blk>>buf>>num_terminal;

    for(int i = 0; i < num_block; i++){
        input_blk>>name>>w>>h;
        Block block(name, w, h);
        block_list.push_back(block);
    }

    for(int i = 0; i < num_block; i++){
        name2block.insert(pair<string, Block*>(block_list[i].getName(), &block_list[i]));
    }

    for(int i = 0; i < num_terminal; i++){
        input_blk>>name>>buf>>w>>h;
        Terminal terminal(name, w, h);
        terminal_list.push_back(terminal);
    }

    for(int i = 0; i < num_terminal; i++){
        name2terminal.insert(pair<string, Terminal*>(terminal_list[i].getName(), &terminal_list[i]));
    }
}

void Floorplanner::read_net(fstream& input_net){
    string buf, name;
    size_t degree;
    input_net>>buf>>num_net;
    for(int i = 0; i < num_net; i++){
        Net net;
        input_net>>buf>>degree;
        for(int j = 0; j < degree; j++){
            input_net>>name;
            net.addTerm(name);
        }

        net_list.push_back(net);
     }
}

double Floorplanner::HPWL() {  
    double totalHPWL = 0;
    for (int j = 0; j < num_net; j++) {
        vector<string> termlist = net_list[j].getTermList();
        double x_min, x_max, y_min, y_max, x, y;
        if(name2block.find(termlist[0]) != name2block.end()){
            Block *block = name2block.at(termlist[0]);
            x = block->getX()+(double)(block->getWidth()/2); 
            y = block->getY()+(double)(block->getHeight()/2);
        }
        else{
            Terminal *terminal = name2terminal.at(termlist[0]);
            x = terminal->getX(); 
            y = terminal->getY();
        }
        x_min = x;
        x_max = x;
        y_min = y;
        y_max = y;
        for (int i = 1; i < termlist.size(); i++) {
            if(name2block.find(termlist[i]) != name2block.end()){
                Block *block = name2block.at(termlist[i]);
                x = block->getX()+(double)(block->getWidth()/2); 
                y = block->getY()+(double)(block->getHeight()/2);
            }
            else{
                Terminal *terminal = name2terminal.at(termlist[i]);
                x = terminal->getX(); 
                y = terminal->getY();
            }
            if(x < x_min) x_min = x;
            if(y < y_min) y_min = y;
            if(x > x_max) x_max = x;
            if(y > y_max) y_max = y;
        }
        totalHPWL += ((x_max - x_min) + (y_max - y_min));
    }
    return totalHPWL;
}

int Floorplanner::Box_Area() {
    return max_x * max_y;
}

double Floorplanner::Cost(double alpha = 0.13) {
    int A = Box_Area();
    int W = HPWL();
    return alpha * A + (1 - alpha) * W;
}

void Floorplanner::floorplan(fstream& output){
    auto start = chrono::high_resolution_clock::now();
    mt19937 gen(145066);

    double best_cost = -1;
    while(best_cost == -1){
        //init solution
        
        for(int i = 0; i < block_list.size(); i++){
            tree.insert(&block_list[i]);
        }
        //SA

        //init variable
        int k = 15;//constant 15
        int n = num_block;
        int N = k * n;//max attempt

        int M = 0, MT = 1, uphill = 0;//chosen move, move times, uphill times, reject times
        double T = 1200;//init T 1200
        double Tmin = 0.0001;//terminate T
        double R = 0.9;//cool rate;

        //random
        uniform_int_distribution<> random_MoveType(0, 2);
        uniform_real_distribution<> random_Prob(0.0, 1.0);
        uniform_int_distribution<> random_Node(0, num_block-1);
        uniform_int_distribution<> random_Pos(0, 1);

        packing(tree);
        //init cost
        double old_cost = Cost();
        old_tree = tree;

        if((max_x <= box_width) && (max_y <= box_height))
            best_cost = old_cost;
            best = tree;

        while (T > Tmin) {
            MT = uphill = 0;
            while (MT < 2 * N && uphill < N) {
                int moveType = random_MoveType(gen), node1, node2, pos;
                switch (moveType) {
                    case 0: //rotate
                        node1 = random_Node(gen);
                        tree.rotate(tree.list[node1]);
                        break;
                    case 1: {//remove & insert
                        node1 = random_Node(gen), node2 = random_Node(gen), pos = random_Pos(gen);
                        Block *block = tree.list[node1]->block;
                        node1 = tree.remove(tree.list[node1]); 
                        while(node1 == node2){
                            node2 = random_Node(gen);
                        }
                        Node *new_node = tree.insertNode(block, tree.list[node2], pos);
                        tree.list.clear();
                        tree.build_list(tree.root.get());
                        break;
                    }
                    case 2: //swap
                        node1 = random_Node(gen), node2 = random_Node(gen);
                        while(node1 == node2){
                            node2 = random_Node(gen);
                        }
                        swap(tree.list[node1],tree.list[node2]);
                        tree.swap(tree.list[node1], tree.list[node2]); 
                        break;
                }
            
                packing(tree);
                
                MT++;
                double now_cost = Cost();
                double dCost = now_cost - old_cost;

                if ((max_x <= box_width) && (max_y <= box_height) && (best_cost == -1 || (now_cost < best_cost))) {
                    best = tree;
                    best_cost = now_cost;
                }
                if (dCost <= 0 || exp(-dCost / T) > random_Prob(gen)) {
                    if (dCost > 0) uphill++;
                    old_cost = now_cost;
                    old_tree = tree;
                } else {
                    now_cost = old_cost;
                    tree = old_tree;
                }
            }

            T = T * R;
        }
    }
    packing(best);
    
    output<<best_cost<<endl;
    output<<HPWL()<<endl;
    output<<Box_Area()<<endl;
    output<<max_x<<" "<<max_y<<endl;
    auto now = chrono::high_resolution_clock::now();
    auto elapsed = chrono::duration_cast<chrono::seconds>(now - start);
    output<<elapsed.count()<<endl;
    for(int i = 0; i < num_block; i++){
        output<<block_list[i].getName()<<" "<<block_list[i].getX()<<" "<<block_list[i].getY()<<" "<<(block_list[i].getX()+block_list[i].getWidth())<<" "<<(block_list[i].getY()+block_list[i].getHeight())<<endl;
    }

}

void Floorplanner::packing(BStarTree tree){
    Node *root = tree.root.get();
    if (!root) return;

    head = new ContourNode{0, 0, nullptr};
    max_x = 0;
    max_y = 0;

    placeBlock(root, 0);

    while (head) {
        ContourNode* toDelete = head;
        head = head->next;
        delete toDelete;
    }
}

void Floorplanner::placeBlock(Node* node, int x) {
    if(!node) return;
    ContourNode *curr = head;
    int i = 0;

    if (!node) return;

    int y = 0;
    curr = head;
    while (curr && curr->next) {
        if((curr->x < (x + node->block->getWidth())) && (curr->next->x > x))
            y = max(y, curr->y);
        curr = curr->next;
    }
    if(curr->x < (x + node->block->getWidth()))
        y = max(y, curr->y);

    node->block->setPos(x, y);

    max_x = max(max_x, x + node->block->getWidth());
    max_y = max(max_y, y + node->block->getHeight());

    curr = head;
    if(curr->x == x)
        curr->y = int(y+node->block->getHeight());
    else{
        while (curr && curr->x < x) {
            if (!curr->next || curr->next->x > x) {
                assert(curr->y < (y + node->block->getHeight()));
                ContourNode* newNode = new ContourNode(x, int(y+node->block->getHeight()), curr->next);
                curr->next = newNode;
                curr = curr->next;
                break;
            }
            else if(curr->next->x == x){
                curr->next->y = y + node->block->getHeight();
                curr = curr->next;
                break;
            }
            curr = curr->next;
        }
    }
        

    if(!curr->next || curr->next->x > (x + node->block->getWidth())){
        ContourNode* newNode = new ContourNode(int(x + node->block->getWidth()), y, curr->next);
        curr->next = newNode;
    }
    else if(curr->next->x < (x + node->block->getWidth())){
        ContourNode* prev = curr;
        curr = curr->next;
        while (curr && curr->x < (x + node->block->getWidth())){
            if (!curr->next || curr->next->x > (x + node->block->getWidth())) {
                ContourNode* newNode = new ContourNode(int(x + node->block->getWidth()), curr->y, curr->next);
                prev->next = newNode;
                delete curr;
                break;
            }
            else if(curr->next->x == (x + node->block->getWidth())){
                prev->next = curr->next;
                delete curr;
                break;
            }
            ContourNode* toDelete = curr;
            curr = curr->next;
            delete toDelete;
        }
    }

    placeBlock(node->left.get(), x + node->block->getWidth());
    placeBlock(node->right.get(), x);
}