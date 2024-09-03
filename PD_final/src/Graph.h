#include <vector>
#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <string>
#include <set>
#include <unordered_set>

using namespace std;

using Blk = DefParser::Block;
using Net = ConnectionJsonParser::NetInfo;
using E_blk = pair<Blk*, Blk*>;

class Graph;
class Node;
class Edge;

template<typename T>
T dist_Mah(const pair<T, T>& a, const pair<T, T>& b) {
    return abs(a.first - b.first) + abs(a.second - b.second);
}

class Edge {
    friend class Graph;
    public:
        Edge(Node* s, Node* e) {
            _start_n = s;
            _end_n = e;
            _len = get_dist();

            _dense = 0;
            _max_dense = 20; // to modify
        }
        void print();
        void set_len(double len) { _len = len; }
        void set_len() { _len = get_dist(); }
        void set_dense(double d) { _dense = d; }
        void add_dense(double d) { _dense += d; }
        double get_dist();
        Node* get_s() { return _start_n; }
        Node* get_e() { return _end_n; }
        float get_w() { 
            if (_is_MST) return _len;
            else return _len; // need consider dense
        }
        E_blk get_egde_blk();
    private:
        bool _is_MST = true;
        Node* _start_n;
        Node* _end_n;
        float _len;
        float _dense;
        float _max_dense;
        // bool _is_add_dij;
        void reset() {
            _dense = 0;
            // _is_add_dij = false;
        }
};

class Node {
    friend class Graph;
    public:
        Node(Blk* blk) {
            // _prev_node = 0;
            _blk = blk;
            _x = blk->position.first;
            _y = blk->position.second;

            _ptDS = this;
            _rank = 1;

            _prev_e = nullptr;
            _val_dij = -1; // -1 = inf
            _is_add_dij = false;
            _heap_id = -1;
        }

        Node(float x, float y) {
            _x = x;
            _y = y;

            _ptDS = this;
            _rank = 1;

            _prev_e = nullptr;
            _val_dij = -1; // -1 = inf
            _is_add_dij = false;
            _heap_id = -1;
        }
        void print();
        Node* getNeighbor(Edge* e);
        pair<float, float> get_coord() {
            return {_x, _y};
        }
        bool operator > (const Node v) const {
            if (_val_dij == v._val_dij) return false;
            else if (_val_dij == -1) return true;
            else if (v._val_dij == -1) return false;
            else return _val_dij > v._val_dij;
        }
        Blk* get_blk() { return _blk; }
    private:
        float _x;
        float _y;
        vector<Edge*> _e_list;
        void push_e(Edge* e) {
            if (getNeighbor(e) != nullptr) _e_list.push_back(e);
        }
        void reset() {
            _ptDS = this;
            _rank = 1;
            _prev_e = nullptr;
            _val_dij = -1;
            _is_add_dij = false;
        }

        // for disjoint set
        // modified in Graph
        Node* _ptDS;
        int _rank;

        // for Dijkstra
        Edge* _prev_e;
        float _val_dij;
        bool _is_add_dij;
        int _heap_id;
        void Dijk_addv() {
            _val_dij = 0;
            _is_add_dij = true;
        }
        Blk* _blk;

};

class Graph {
    public:
        Graph() {}
        void push_v(Blk* blk) {
            Node* v = new Node(blk);
            push_v(v);
        }
        void push_v(Node* v) {
            V.push_back(v);
        }
        void push_e(Edge* e) {
            E.push_back(e);
            e->get_s()->push_e(e);
            e->get_e()->push_e(e);
        }
        void make_complete_g() {
            for (size_t i = 0; i < V.size(); i++) {
                for (size_t j = i + 1; j < V.size(); j++) {
                    push_e(new Edge(V[i], V[j]));
                }
            }
        }
        void reset() {
            reset_V();
            reset_E();
        }
        void set_Dijk() { 
            reset_V();
            v_remain.clear();
        }
        void reset_V() {
            for (auto& v: V) v->reset();
        }
        void reset_E() {
            for (auto& e: E) e->reset();
        }

        vector<E_blk> MST();
        vector<Edge*> Dijk(unordered_set<Node*>);

        void test();
    private:
        Node* DS_Find (Node*);
        bool DS_Union (Node*, Node*);

        vector<Node*> v_remain;
        Node* HP_ExtractMin();
        void HP_DecreaseKey(Node*, float);
        void HP_Insert(Node*);
        void HP_Heapify(Node*);
        void HP_swap(Node*, Node*);
        Node* HP_par(Node* v) { 
            if (v->_heap_id <= 0) return 0;
            else return v_remain[(v->_heap_id - 1)/2];
        }
        Node* HP_left(Node* v) { 
            if ((2 * v->_heap_id + 1) >= v_remain.size()) return 0;
            return v_remain[(2 * v->_heap_id + 1)];
        }
        Node* HP_right(Node* v) {
            if ((2 * v->_heap_id + 2) >= v_remain.size()) return 0;
            return v_remain[(2 * v->_heap_id + 2)];
        }
        void Dijk_Relax(Node*);

        vector<Node*> V;
        vector<Edge*> E;
};