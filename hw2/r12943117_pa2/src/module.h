#ifndef MODULE_H
#define MODULE_H

#include <vector>
#include <string>
#include <memory>
#include <random>
using namespace std;

namespace my {
    template<typename T, typename... Args>
    std::unique_ptr<T> make_unique(Args&&... args) {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }
}

class Terminal
{
public:
    // constructor and destructor
    Terminal(const string& name, size_t x, size_t y) :
        _name(name), _x(x), _y(y) { }
    virtual ~Terminal()  { }

    // basic access methods
    const string getName()  { return _name; }
    const double getX()    { return _x; }
    const double getY()    { return _y; }

    // set functions
    void setName(string& name) { _name = name; }
    void setPos(size_t x1, size_t y1) {
        _x = x1;   _y = y1;
    }

private:
    string      _name;     // module name
    double      _x;        // min x coordinate of the terminal
    double      _y;        // min y coordinate of the terminal
};

class Node;

class Block
{
public:
    // constructor and destructor
    Block(const string& name, size_t w, size_t h) :
        _name(name), _x(0), _y(0), _w(w), _h(h), _rotate(0) { }
    ~Block() { }

    // basic access methods
    const double getWidth(bool rotate = false)  { return rotate? _h: _w; }
    const double getHeight(bool rotate = false) { return rotate? _w: _h; }
    const size_t getArea()  { return _h * _w; }
    const size_t getRotate()  { return _rotate; }

    // basic access methods
    const string getName()  { return _name; }
    const double getX()    { return _x; }
    const double getY()    { return _y; }

    // set functions
    void setName(string& name) { _name = name; }
    void setPos(double x1, double y1) {
        _x = x1;   _y = y1;
    }

    // set functions
    void setWidth(double w)         { _w = w; }
    void setHeight(double h)        { _h = h; }
    void setRotate()                { _rotate ^= 1; }

private:
    double          _w;         // width of the block
    double          _h;         // height of the block
    bool            _rotate;    
    string          _name;     // module name
    double          _x;        // min x coordinate of the terminal
    double          _y;        // min y coordinate of the terminal
    
};


class Net
{
public:
    // constructor and destructor
    Net()   { }
    ~Net()  { }

    // basic access methods
    const vector<string> getTermList()   { return _termList; }

    // modify methods
    void addTerm(string name) { _termList.push_back(name); }

private:
    vector<string>   _termList;  // list of terminals the net is connected to
};

class Node {
public:
    Block* block;
    unique_ptr<Node> left;
    unique_ptr<Node> right;
    Node* parent;
    
    Node(Block* blk) : block(blk), parent(nullptr) {}
};

class BStarTree {
public:

    vector<Node*> list;
    unique_ptr<Node> root;
    mt19937 gen;

    BStarTree() {
        gen.seed(2790);//123 2790
        root = nullptr;
        list.clear();
    }
    ~BStarTree() {}

    // Operations
    void insert(Block* block);
    Node* insertNode(Block* newBlock, Node* parentNode, bool insertLeft);
    int remove(Node* node);
    void rotate(Node* node);
    void swap(Node* a, Node* b);

    //copy
    BStarTree& operator=(const BStarTree& other) {
        if (this != &other) {
            BStarTree temp(other);
            std::swap(root, temp.root);
            list.clear();   
            build_list(root.get());     
        }
        return *this;
    }

    BStarTree(const BStarTree& other) {
        list.clear();
        root = copyTree(other.root.get(), nullptr);
    }

    unique_ptr<Node> copyTree(const Node* sourceNode, Node* parentNode) {
        if (!sourceNode) return nullptr;

        unique_ptr<Node> newNode = my::make_unique<Node>(sourceNode->block);
        newNode->parent = parentNode;
        newNode->left = copyTree(sourceNode->left.get(), newNode.get());
        newNode->right = copyTree(sourceNode->right.get(), newNode.get());
        return newNode;
    }

    void build_list(Node *node) {
        if (!node) return;
        list.push_back(node);
        build_list(node->left.get());
        build_list(node->right.get());
    }

};




#endif  // MODULE_H