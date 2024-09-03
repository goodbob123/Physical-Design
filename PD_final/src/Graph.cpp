#include <Router.h>
#include <Graph.h>
#include <assert.h>

// some useful function

void Graph::test() {
    // V = vector<Node*> ();
    // E = vector<Edge*> ();
    // push_v(new Node(0, 1));
    // push_v(new Node(2, 1));
    // push_v(new Node(3, 0));
    // push_v(new Node(4, 3));
    // push_v(new Node(0, 0));
    // make_complete_g();
    // MST();
    // for (auto e: Dijk(unordered_set<Node* >(V.begin(), V.end()))) {
    //     e->print();
    // }
    // for (auto e: MST()) {
    //     e->print();
    // }
}

vector<E_blk> Graph::MST() {
    vector<Edge*> E_buf(E);
    vector<Edge*> mst;
    sort(E_buf.begin(), E_buf.end(), [](Edge* & a, Edge* & b) { 
        return a->get_w() < b->get_w(); 
    });
    for (size_t i = 0; i < E_buf.size() && mst.size() < V.size() - 1; i++){
        Edge* edge = E_buf[i];
        if (DS_Union(edge->_start_n, edge->_end_n)) {
            mst.push_back(edge);
        }
    }
    reset();
    assert(mst.size() == V.size() - 1);
    vector<E_blk> ans;
    for (auto e: mst) {
        ans.push_back(e->get_egde_blk());
    }
    return ans;
}

Node* Graph::DS_Find (Node* v) {
    if (v->_ptDS != v) v->_ptDS = DS_Find(v->_ptDS);
    return v->_ptDS;
}
bool Graph::DS_Union (Node* u, Node* v) {
    Node* u_lead = DS_Find(u);
    Node* v_lead = DS_Find(v);
    if (u_lead != v_lead) {
        if (u_lead->_rank < v_lead->_rank) u_lead->_ptDS = v_lead;
        else if (u_lead->_rank > v_lead->_rank) v_lead->_ptDS = u_lead;
        else {
            u_lead->_ptDS = v_lead;
            v_lead->_rank++;
        }
        return true;
    } else {
        return false;
    }
}

vector<Edge*> Graph::Dijk(unordered_set<Node*> v_net) {
    set_Dijk();
    vector<Edge*> wiring;

    Node* v0 = *(v_net.begin());
    v0->_val_dij = 0;
    v0->_prev_e = 0;

    for (auto& v: V) {
        HP_Insert(v);
    }
    while (!v_net.empty() && !v_remain.empty()) {
        Node* cur_v = HP_ExtractMin();
        auto cur_v_iter = v_net.find(cur_v);
        if (cur_v_iter != v_net.end()) {
            // we found a new RX
            v_net.erase(cur_v_iter);
            while (!cur_v->_is_add_dij) {
                cur_v->Dijk_addv();
                Dijk_Relax(cur_v);
                if (cur_v->_prev_e != nullptr) { // v0 has no predecessor
                    //assert(cur_v->_prev_e != nullptr);
                    wiring.push_back(cur_v->_prev_e);
                    cur_v = cur_v->getNeighbor(cur_v->_prev_e);
                }
            }
        } else {
            // update neighbor
            Dijk_Relax(cur_v);
        }
    }
    assert(v_net.empty());
    return wiring;
}
Node* Graph::HP_ExtractMin() {
    if (v_remain.empty()) return 0;
    if (v_remain.size() == 1) {
        Node* min_v = v_remain[0];
        min_v->_heap_id = -1;
        v_remain.clear();
        return min_v;
    }
    
    HP_swap(v_remain.front(), v_remain.back());
    Node* min_v = v_remain.back();
    min_v->_heap_id = -1;
    
    v_remain.pop_back();
    HP_Heapify(v_remain[0]);
    return min_v;
}
void Graph::HP_DecreaseKey(Node* v, float val) {
    if (v->_is_add_dij) return;
    assert(v->_val_dij >= val || v->_val_dij == -1);

    size_t id = v->_heap_id;
    v->_val_dij = val;
    if (v->_heap_id == -1) HP_Insert(v);
    else {
        while (HP_par(v) != 0 && *HP_par(v) > *v) { 
            HP_swap(v, HP_par(v));
        }
    }
}
void Graph::HP_Insert(Node* v) {
    v_remain.push_back(v);
    v->_heap_id = v_remain.size() - 1;
    while (HP_par(v) != 0 && *HP_par(v) > *v) { 
       HP_swap(v, HP_par(v));
    }
}
void Graph::HP_Heapify(Node* v) {
    Node* l = HP_left(v); 
    Node* r = HP_right(v); 
    Node* smallest = v; 
    if (l != 0 && *v > *l) smallest = l; 
    if (r != 0 && *smallest > *r) smallest = r; 
    if (smallest != v) { 
        HP_swap(v, smallest); 
        HP_Heapify(smallest); 
    } 
}
void Graph::HP_swap(Node* u, Node* v) {
    // swap(u, v);
    int temp = u->_heap_id;
    u->_heap_id = v->_heap_id;
    v->_heap_id = temp;
    v_remain[u->_heap_id] = u;
    v_remain[v->_heap_id] = v;
}
void Graph::Dijk_Relax(Node* v) {
    for (Edge* e: v->_e_list) {
        Node* u = v->getNeighbor(e);
        float new_dist = v->_val_dij + e->get_w();
        if (u->_val_dij == -1 || u->_val_dij > new_dist) {
            u->_prev_e = e;
            HP_DecreaseKey(u, new_dist);
        }
    }
}
// edge
void Edge::print() {
    cout << "(" << _start_n->get_coord().first << "," << _start_n->get_coord().second << ") ~ "
        << "(" << _end_n->get_coord().first << "," << _end_n->get_coord().second << ") --> "
        << get_w() << endl;
}
double Edge::get_dist() {
    if (_is_MST) {
        return dist_Mah(_start_n->get_coord(), _end_n->get_coord());
    } else {
        return dist_Mah(_start_n->get_coord(), _end_n->get_coord());
    }
}
E_blk Edge::get_egde_blk() {
    return {_start_n->get_blk(), _end_n->get_blk()};
}
// Node
// int Node::_global_flag = 0;
void Node::print() {
}
Node* Node::getNeighbor(Edge* e) {
    assert(e != nullptr);
    if (e->get_s() == this) return e->get_e();
    else if (e->get_e() == this) return e->get_s();
    else return 0;
}

// bool Node::reachable(Node* v, bool set) {
//     if (v == this) return true;
//     if (set) setGlobalFlag();
//     _flag = _global_flag;
//     vector<Node*> neighbor;
//     for (auto& e: e_list) {
//         if (e->_removed) continue;
//         Node* n = getNeighbor(e);
//         if (n->_flag != _global_flag) neighbor.push_back(n);
//     }
//     for (auto& n: neighbor) {
//         if (n->reachable(v, false)) return true;
//     }
//     return false;
// }