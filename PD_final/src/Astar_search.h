#include <vector>
#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <map>
#include <string>

#define K 1

using namespace std;

int cal_h(int x, int y, int stop_x, int stop_y);
vector<pair<int, int>> astar_search(Router router, int origin_grid_x, int origin_grid_y, int stop_grid_x, int stop_grid_y);
vector<pair<int, int>> Z_shape(Router& router, int origin_grid_x, int origin_grid_y, int stop_grid_x, int stop_grid_y);
bool check_overlap(int x01, int x02, int x11, int x12) {
    return (x01 < x11 && x11 < x02) || (x01 < x12 && x12 < x02);
}
int comp_sec_cost(Grid** global_g, int origin_grid_x, int origin_grid_y, int stop_grid_x, int stop_grid_y) {
    // cout << origin_grid_x << " " << origin_grid_y << " " << stop_grid_x << " " << stop_grid_y << endl;
    int cost = 0;
    if (origin_grid_x == stop_grid_x) {
        int fix = origin_grid_x;
        int small = min(origin_grid_y, stop_grid_y);
        int big = max(origin_grid_y, stop_grid_y);
        for (int k = small; k <= big; k++){
            if (global_g[fix][k].get_throughable() == false) return -1;
            else cost += global_g[fix][k].get_wirenum();
        }
    } else if (origin_grid_y == stop_grid_y){
        int fix = origin_grid_y;
        int small = min(origin_grid_x, stop_grid_x);
        int big = max(origin_grid_x, stop_grid_x);
        for (int k = small; k <= big; k++){
            if (global_g[k][fix].get_throughable() == false) return -1;
            else cost += global_g[k][fix].get_wirenum();
        }
    }
    return cost;
}
int comp_cost(Grid** global_g, vector<pair<int, int>>& pt) {
    int cost = 0;
    for (int i = 1; i < pt.size(); i++) {
        int c_sec = comp_sec_cost(global_g, pt[i - 1].first, pt[i - 1].second, pt[i].first, pt[i].second);
        if (c_sec < 0) {
            // cout << "fail" << endl;
            return -1;
        }
        else cost += c_sec;
    } 
    return cost;
}

vector<pair<int, int>> astar_search(Router router, int origin_grid_x, int origin_grid_y, int stop_grid_x, int stop_grid_y){
    // test
    // cerr << "start " << "x: " << origin_grid_x << " y: " << origin_grid_y << endl;
    // cerr << "end " << "x: " << stop_grid_x << " y: " << stop_grid_y << endl;
    vector<Grid*> Grid_list;
    // initial cur_grid
    Grid* cur_grid = new Grid(0, 0, origin_grid_x, origin_grid_y, 0);
    cur_grid->set_explored(1);

    Grid* nxt_grid = nullptr;

    // record path for output
    vector<pair<int, int>> path_grid_list;

    int right_cost, left_cost, up_cost, down_cost;
    Grid* right_grid;
    Grid* left_grid;
    Grid* up_grid;
    Grid* down_grid;

    // initial
    if (((origin_grid_x + 1) < router.grid_index(router.getBoundingbox()).first+1) && (router.grid_graph[origin_grid_x + 1][origin_grid_y].get_throughable() == true)){
        // right_cost = 1 + cal_h(origin_grid_x + 1, origin_grid_y, stop_grid_x, stop_grid_y);
        right_cost = 1 + cal_h(origin_grid_x + 1, origin_grid_y, stop_grid_x, stop_grid_y) + router.grid_graph[origin_grid_x + 1][origin_grid_y].get_wirenum() * K;
        right_grid = new Grid(1, right_cost, origin_grid_x + 1, origin_grid_y, 0);
        right_grid->set_prev(cur_grid);
        // set_explored
        router.grid_graph[origin_grid_x + 1][origin_grid_y].set_explored(1);
        Grid_list.push_back(right_grid);
    }

    if (((origin_grid_x - 1) > -1) && (router.grid_graph[origin_grid_x - 1][origin_grid_y].get_throughable() == true)){
        // left_cost = 1 + cal_h(origin_grid_x - 1, origin_grid_y, stop_grid_x, stop_grid_y);
        left_cost = 1 + cal_h(origin_grid_x - 1, origin_grid_y, stop_grid_x, stop_grid_y) + router.grid_graph[origin_grid_x - 1][origin_grid_y].get_wirenum() * K;
        left_grid = new Grid(1, left_cost, origin_grid_x - 1, origin_grid_y, 0);
        left_grid->set_prev(cur_grid);
        // set_explored
        router.grid_graph[origin_grid_x - 1][origin_grid_y].set_explored(1);
        Grid_list.push_back(left_grid);
    }

    if (((origin_grid_y + 1) < router.grid_index(router.getBoundingbox()).second+1) && (router.grid_graph[origin_grid_x][origin_grid_y + 1].get_throughable() == true)){
        // up_cost = 1 + cal_h(origin_grid_x, origin_grid_y + 1, stop_grid_x, stop_grid_y);
        up_cost = 1 + cal_h(origin_grid_x, origin_grid_y + 1, stop_grid_x, stop_grid_y) + router.grid_graph[origin_grid_x][origin_grid_y + 1].get_wirenum() * K;
        up_grid = new Grid(1, up_cost, origin_grid_x, origin_grid_y + 1, 0);
        up_grid->set_prev(cur_grid);
        // set_explored
        router.grid_graph[origin_grid_x][origin_grid_y + 1].set_explored(1);
        Grid_list.push_back(up_grid);
    }

    if (((origin_grid_y - 1) > -1) && (router.grid_graph[origin_grid_x][origin_grid_y - 1].get_throughable() == true)){
        // down_cost = 1 + cal_h(origin_grid_x, origin_grid_y - 1, stop_grid_x, stop_grid_y);
        down_cost = 1 + cal_h(origin_grid_x, origin_grid_y - 1, stop_grid_x, stop_grid_y) + router.grid_graph[origin_grid_x][origin_grid_y - 1].get_wirenum() * K;
        down_grid = new Grid(1, down_cost, origin_grid_x, origin_grid_y - 1, 0);
        down_grid->set_prev(cur_grid);
        // set_explored
        router.grid_graph[origin_grid_x][origin_grid_y - 1].set_explored(1);
        Grid_list.push_back(down_grid);
    }

    // Grid_list: all the grid that can possibly be chose, pop out if chosen
    int min_cost = 10000000;
    int min_h = 10000000;
    for (int i = 0; i < Grid_list.size(); i++){
        if ((Grid_list[i]->get_cost() <= min_cost) && (Grid_list[i]->get_cost() - Grid_list[i]->get_G() < min_h)){
            min_cost = Grid_list[i]->get_cost();
            min_h = Grid_list[i]->get_cost() - Grid_list[i]->get_G();
            nxt_grid = Grid_list[i];
        }
    }

    // test segmentation
    if (nxt_grid == nullptr){
        return path_grid_list;
    }


    // update cur_grid
    cur_grid = nxt_grid;

    // pop out
    vector<Grid*>::iterator it = find(Grid_list.begin(), Grid_list.end(), nxt_grid);
    if (it != Grid_list.end()) {
        Grid_list.erase(it);
    }



    // loop
    int count = 0;
    while(!((cur_grid->get_x() == stop_grid_x) && (cur_grid->get_y() == stop_grid_y))){
    // for (int i = 0; i < 5; i++){
        //cerr << "-------------iteration "<<count++<<"----------------" << endl;
        if (cur_grid->get_prev() == nullptr) {
            break;
        }
        int prev_x = cur_grid->get_prev()->get_x();
        int prev_y = cur_grid->get_prev()->get_y();
        
        int cur_x = cur_grid->get_x();
        int cur_y = cur_grid->get_y();

        // avoid backtrack (dont cal when obstacle or it is prev block)
        if ( (!((cur_x + 1 == prev_x) && (cur_y == prev_y))) && ((cur_x + 1) < router.grid_index(router.getBoundingbox()).first+1) && (router.grid_graph[cur_x + 1][cur_y].get_throughable() == true) && (router.grid_graph[cur_x + 1][cur_y].get_explored() == false)){
            // right_cost = cur_grid->get_G() + 1 + cal_h(cur_x + 1, cur_y, stop_grid_x, stop_grid_y);
            right_cost = cur_grid->get_G() + 1 + cal_h(cur_x + 1, cur_y, stop_grid_x, stop_grid_y) + router.grid_graph[cur_x + 1][cur_y].get_wirenum() * K;
            right_grid = new Grid(cur_grid->get_G() + 1, right_cost, cur_x + 1, cur_y, 0);
            right_grid->set_prev(cur_grid);
            // set_explored
            router.grid_graph[cur_x + 1][cur_y].set_explored(1);
            Grid_list.push_back(right_grid);
        }

        if ( (!((cur_x - 1 == prev_x) && (cur_y == prev_y))) && ((cur_x - 1) > -1) && (router.grid_graph[cur_x - 1][cur_y].get_throughable() == true)  && (router.grid_graph[cur_x - 1][cur_y].get_explored() == false) ){
            // left_cost = cur_grid->get_G() + 1 + cal_h(cur_x - 1, cur_y, stop_grid_x, stop_grid_y);
            left_cost = cur_grid->get_G() + 1 + cal_h(cur_x - 1, cur_y, stop_grid_x, stop_grid_y) + router.grid_graph[cur_x - 1][cur_y].get_wirenum() * K;
            left_grid = new Grid(cur_grid->get_G() + 1, left_cost, cur_x - 1, cur_y, 0);
            left_grid->set_prev(cur_grid);
            // set_explored
            router.grid_graph[cur_x - 1][cur_y].set_explored(1);
            Grid_list.push_back(left_grid);
        }
        
        if ( (!((cur_x == prev_x) && (cur_y + 1 == prev_y))) && ((cur_y + 1) < router.grid_index(router.getBoundingbox()).second+1) && (router.grid_graph[cur_x][cur_y + 1].get_throughable() == true) && (router.grid_graph[cur_x][cur_y + 1].get_explored() == false)){
            // up_cost = cur_grid->get_G() + 1 + cal_h(cur_x, cur_y + 1, stop_grid_x, stop_grid_y);
            up_cost = cur_grid->get_G() + 1 + cal_h(cur_x, cur_y + 1, stop_grid_x, stop_grid_y) + router.grid_graph[cur_x][cur_y + 1].get_wirenum() * K;
            up_grid = new Grid(cur_grid->get_G() + 1, up_cost, cur_x, cur_y + 1, 0);
            up_grid->set_prev(cur_grid);
            // set_explored
            router.grid_graph[cur_x][cur_y + 1].set_explored(1);
            Grid_list.push_back(up_grid);
        }

        if ( (!((cur_x == prev_x) && (cur_y - 1 == prev_y))) && ((cur_y - 1) > -1) && (router.grid_graph[cur_x][cur_y - 1].get_throughable() == true) && (router.grid_graph[cur_x][cur_y - 1].get_explored() == false)){
            // down_cost = cur_grid->get_G() + 1 + cal_h(cur_x, cur_y - 1, stop_grid_x, stop_grid_y);
            down_cost = cur_grid->get_G() + 1 + cal_h(cur_x, cur_y - 1, stop_grid_x, stop_grid_y) + router.grid_graph[cur_x][cur_y - 1].get_wirenum() * K;
            down_grid = new Grid(cur_grid->get_G() + 1, down_cost, cur_x, cur_y - 1, 0);
            down_grid->set_prev(cur_grid);
            // set_explored
            router.grid_graph[cur_x][cur_y - 1].set_explored(1);
            Grid_list.push_back(down_grid);
        }

        min_cost = 10000000;
        min_h = 10000000;
        for (int i = 0; i < Grid_list.size(); i++){
            if ((Grid_list[i]->get_cost() <= min_cost) && (Grid_list[i]->get_cost() - Grid_list[i]->get_G() < min_h)){
                min_cost = Grid_list[i]->get_cost();
                min_h = Grid_list[i]->get_cost() - Grid_list[i]->get_G();
                nxt_grid = Grid_list[i];
            }
        }
       

        // update cur_grid
        cur_grid = nxt_grid;


        // pop out
        vector<Grid*>::iterator it = find(Grid_list.begin(), Grid_list.end(), nxt_grid);
        if (it != Grid_list.end()) {
            Grid_list.erase(it);
        }

        
        // test
        if (Grid_list.size() == 0){
            break;
        }
    }

    
    while(1){
        if ((cur_grid->get_x() == origin_grid_x) && (cur_grid->get_y() == origin_grid_y)) break;
        path_grid_list.push_back({cur_grid->get_x(), cur_grid->get_y()});
        // record wirenum
        router.grid_graph[cur_grid->get_x()][cur_grid->get_y()].inc_wirenum();
        cur_grid = cur_grid->get_prev();
    }
    
    // free Grid_list
    for (int i = 0; i < Grid_list.size(); i++){
        delete Grid_list[i];
    }
    Grid_list.clear();

    // reset grid_graph
    for (int i = 0; i < router.grid_index(router.getBoundingbox()).first+1; i++){
        for(int j = 0; j < router.grid_index(router.getBoundingbox()).second+1; j++){
            router.grid_graph[i][j].reset();
        }
    }

    return path_grid_list;
}

vector<pair<int, int>> Z_shape(Router& router, int origin_grid_x, int origin_grid_y, int stop_grid_x, int stop_grid_y) {
    Grid** global_g = router.grid_graph;
    vector<vector<pair<int, int>>> result;
    vector<pair<int, int>> best_path;
    int x_span = abs(origin_grid_x - stop_grid_x);
    int y_span = abs(origin_grid_y - stop_grid_y);
    int x_min = min(origin_grid_x, stop_grid_x);
    int y_min = min(origin_grid_y, stop_grid_y);

    int cost;
    int min_cost = -1;
    int best_id = -1;
    const int trial = 2;

    if (x_span == 0 || y_span == 0) {
        result.push_back({{origin_grid_x, origin_grid_y}, {stop_grid_x, stop_grid_y}});
        cost = comp_cost(global_g, result.back());
        if (cost > 0) {
            if (min_cost == -1 || min_cost > cost) {
                min_cost = cost;
                best_id = result.size() - 1;
            }
        }
    } else {
        result.push_back({{origin_grid_x, origin_grid_y}, {stop_grid_x, origin_grid_y}, {stop_grid_x, stop_grid_y}});
        cost = comp_cost(global_g, result.back());
        if (cost > 0) {
            if (min_cost == -1 || min_cost > cost) {
                min_cost = cost;
                best_id = result.size() - 1;
            }
        }
        // |-
        result.push_back({{origin_grid_x, origin_grid_y}, {origin_grid_x, stop_grid_y}, {stop_grid_x, stop_grid_y}});
        cost = comp_cost(global_g, result.back());
        if (cost > 0) {
            if (min_cost == -1 || min_cost > cost) {
                min_cost = cost;
                best_id = result.size() - 1;
            }
        }
        // Z
        for (int i = 1; i < trial + 1; i++) {
            int trial_x = x_min + rand() % x_span;
            result.push_back({{origin_grid_x, origin_grid_y}, {trial_x, origin_grid_y}, {trial_x, stop_grid_y}, {stop_grid_x, stop_grid_y}});
            cost = comp_cost(global_g, result.back());
            if (cost > 0) {
                if (min_cost == -1 || min_cost > cost) {
                    min_cost = cost;
                    best_id = result.size() - 1;
                }
            }
            int trial_y = y_min + rand() % y_span;
            result.push_back({{origin_grid_x, origin_grid_y}, {origin_grid_x, trial_y}, {stop_grid_x, trial_y}, {stop_grid_x, stop_grid_y}});
            cost = comp_cost(global_g, result.back());
            if (cost > 0) {
                if (min_cost == -1 || min_cost > cost) {
                    min_cost = cost;
                    best_id = result.size() - 1;
                }
            }
        }
    }

    if (best_id == -1) return vector<pair<int, int>> ();
    // cout << best_id << endl;
    best_path = result[best_id];
    // for (auto p: best_path) {
    //     cout << p.first << " " << p.second << endl;
    // }

    // cout << "success" << endl;



    vector<pair<int, int>> output;
    for (int i = 1; i < best_path.size(); i++) {
        int x1 = best_path[i - 1].first;
        int y1 = best_path[i - 1].second;
        int x2 = best_path[i].first;
        int y2 = best_path[i].second;
        // cout << x1 << " " << x2 << " " << y1 << " " << y2 << endl; 
        if (x1 == x2) {
            int fix = x1;
            int cur = y1;
            int end = y2;
            while (cur != end) {
                output.push_back({fix, cur});
                global_g[fix][cur].inc_wirenum();
                cur = cur < end ? cur + 1 : cur - 1;
            }
        } else if (y1 == y2){
            int fix = y1;
            int cur = x1;
            int end = x2;
            while (cur != end) {
                output.push_back({cur, fix});
                global_g[cur][fix].inc_wirenum();
                cur = cur < end ? cur + 1 : cur - 1;
            }
        }
    }
    // cout << "yyy" << endl;
    // for (auto p: output) {
    //     cout << p.first << " " << p.second << endl;
    // }
    return output;
}


int cal_h(int x, int y, int stop_x, int stop_y){
    int x_out = abs(stop_x - x);
    int y_out = abs(stop_y - y);
    return x_out + y_out;
    /*
    if (x_out >= y_out) return x_out;
    else return y_out;
    */
}