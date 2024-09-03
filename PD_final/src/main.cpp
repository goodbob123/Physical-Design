#include <Router.h>
#include <Graph.h>
#include <Astar_search.h>

using namespace std;

int main(int argc, char* argv[]) {

    //start timer
    auto start = chrono::high_resolution_clock::now();

    bool draw = false;
    int tracks_per_um;
    string defDirectory;
    string cfgFile;
    string connectionFile;
    //parser
    if(argc == 6) {
        draw = true;
        tracks_per_um = stoi(argv[2]);
        defDirectory = argv[3];
        cfgFile = argv[4];
        connectionFile = argv[5];
    }
    else if (argc == 5) {
        tracks_per_um = stoi(argv[1]);
        defDirectory = argv[2];
        cfgFile = argv[3];
        connectionFile = argv[4];
    }
    else{
        cerr << "Usage: ./CGR <tracks/um> <def_directory> <cfg_file> <connection_file>" << endl;
        return -1;
    }
    
    DefParser defParser(defDirectory);
    if (!defParser.parse()) {
        cerr << "Failed to parse DEF files." << endl;
        return -1;
    }

    CfgJsonParser cfgParser(cfgFile);
    if (!cfgParser.parse()) {
        cerr << "Failed to parse CFG JSON file." << endl;
        return -1;
    }

    ConnectionJsonParser connectionParser(connectionFile);
    if (!connectionParser.parse()) {
        cerr << "Failed to parse Connection JSON file." << endl;
        return -1;
    }

    Router router(tracks_per_um, connectionParser.getmaxTrack(), defParser.getBoundingbox(), defParser.getBlocks(), cfgParser.getBlocks(), connectionParser.getNets());
    cerr << "parse finish, time = " << (chrono::duration_cast<std::chrono::milliseconds>(chrono::high_resolution_clock::now() - start).count()) / 1000.0 << endl;
    if(draw){
        router.printGrid();
        return 0;
    }

    //output file
    string out_file = string("case") + defDirectory[defDirectory.find("_") - 1] + "_net.rpt";
    ofstream outFile(out_file);

    // Now, the router object contains all the necessary data for further processing.


    // usage: 
    // Graph G;
    // for (block in net) G.push_v(&block)
    // G.make_complete_g()
    // global_wire = G.MST()


    vector<ConnectionJsonParser::NetInfo> net = router.getNets();
    unordered_map<string, DefParser::Block> blocks = router.getBlocks();
    
    // TODO: net-ordering

    for(int i = 0; i < net.size(); i++){
        // new graph
        Graph cur_graph;

        // record path for each net, i: which block pair, j: grids on path
        vector<vector<pair<int, int>>> Astar_out;

        // push_v all blks of this net
        cur_graph.push_v(&blocks[net[i].tx]);

        vector<string> blk_rx_list = net[i].rx;
        for (int j = 0; j < blk_rx_list.size(); j++){
            cur_graph.push_v(&blocks[blk_rx_list[j]]);
        }

        // for test
        //cout << "start MST" << endl;
    
        // implement MST: get vector of pair of blks
        cur_graph.make_complete_g();
        vector<pair<DefParser::Block*, DefParser::Block*>> MST_out = cur_graph.MST();
        
        //cout << "start Astar" << endl;

        // Find coordinates of each pair
        for (int j = 0; j < MST_out.size(); j++){

            // Block*
            DefParser::Block* cur_blk_1 = MST_out[j].first;
            DefParser::Block* cur_blk_2 = MST_out[j].second;

            double cur_blk_1_x, cur_blk_1_y;
            double cur_blk_2_x, cur_blk_2_y;

            // start blk: blk_1, end_blk: blk_2
            if (MST_out[j].first == &blocks[net[i].tx]){
                // tx
                cur_blk_1_x = (MST_out[j].first)->left_bottom.first + net[i].txCoord.first;
                cur_blk_1_y = (MST_out[j].first)->left_bottom.second + net[i].txCoord.second;

                // multiple rx
                int index;

                for (int k = 0; k < net[i].rx.size(); k++){
                    if (MST_out[j].second->name == net[i].rx[k]) index = k;
                }

                cur_blk_2_x = (MST_out[j].second)->left_bottom.first + net[i].rxCoord[index].first;
                cur_blk_2_y = (MST_out[j].second)->left_bottom.second + net[i].rxCoord[index].second;
                
                // change to grid coord
                int grid_1_x, grid_1_y;
                int grid_2_x, grid_2_y;

                grid_1_x = router.grid_index(make_pair(cur_blk_1_x, cur_blk_1_y)).first;
                grid_1_y = router.grid_index(make_pair(cur_blk_1_x, cur_blk_1_y)).second;
                grid_2_x = router.grid_index(make_pair(cur_blk_2_x, cur_blk_2_y)).first;
                grid_2_y = router.grid_index(make_pair(cur_blk_2_x, cur_blk_2_y)).second;

                /*
                // L-shape
                bool left_able = true;
                bool right_able = true;
                bool up_able = true;
                bool down_able = true;

                int min_x = min(grid_1_x, grid_2_x);
                int max_x = max(grid_1_x, grid_2_x);
                int min_y = min(grid_1_y, grid_2_y);
                int max_y = max(grid_1_y, grid_2_y);

                for (int k = min_x; k <= max_x; k++){
                    if (router.grid_graph[k][min_y].get_throughable() == 0) down_able = false;
                    if (router.grid_graph[k][max_y].get_throughable() == 0) up_able = false;
                }
                for (int k = min_y; k <= max_y; k++){
                    if (router.grid_graph[min_x][k].get_throughable() == 0) left_able = false;
                    if (router.grid_graph[max_x][k].get_throughable() == 0) right_able = false;
                }

                // if cant L-shape, then A-star
                if ((down_able == 1) && (right_able == 1)){
                    vector<pair<int, int>> points;
                    
                }
                else if ((left_able == 1) && (up_able == 1)){

                }
                else{
                    // A*-search
                    Astar_out.push_back(astar_search(router, grid_1_x, grid_1_y, grid_2_x, grid_2_y));
                }
                */

                // A*-search
                if (!((grid_1_x == grid_2_x) && (grid_1_y == grid_2_y))){
                    vector<pair<int, int>> L_route = Z_shape(router, grid_1_x, grid_1_y, grid_2_x, grid_2_y);
                    if (!L_route.empty()) {
                        Astar_out.push_back(L_route);
                    } else {
                        Astar_out.push_back(astar_search(router, grid_1_x, grid_1_y, grid_2_x, grid_2_y));
                    }
                }
                else{
                    Astar_out.push_back(vector<pair<int, int>>(2, make_pair(grid_1_x, grid_1_y)));
                }
            }
            // start blk: blk_2, end_blk: blk_1
            else if (MST_out[j].second == &blocks[net[i].tx]){
                cur_blk_2_x = (MST_out[j].second)->left_bottom.first + net[i].txCoord.first;
                cur_blk_2_y = (MST_out[j].second)->left_bottom.second + net[i].txCoord.second;

                // multiple rx
                int index;

                for (int k = 0; k < net[i].rx.size(); k++){
                    if (MST_out[j].second->name == net[i].rx[k]) index = k;
                }
                cur_blk_1_x = (MST_out[j].first)->left_bottom.first + net[i].rxCoord[index].first;
                cur_blk_1_y = (MST_out[j].first)->left_bottom.second + net[i].rxCoord[index].second;

                // change to grid coord
                double grid_1_x, grid_1_y;
                double grid_2_x, grid_2_y;

                grid_1_x = router.grid_index(make_pair(cur_blk_1_x, cur_blk_1_y)).first;
                grid_1_y = router.grid_index(make_pair(cur_blk_1_x, cur_blk_1_y)).second;
                grid_2_x = router.grid_index(make_pair(cur_blk_2_x, cur_blk_2_y)).first;
                grid_2_y = router.grid_index(make_pair(cur_blk_2_x, cur_blk_2_y)).second;

                /*
                // L-shape
                bool left_able = true;
                bool right_able = true;
                bool up_able = true;
                bool down_able = true;

                int min_x = min(grid_1_x, grid_2_x);
                int max_x = max(grid_1_x, grid_2_x);
                int min_y = min(grid_1_y, grid_2_y);
                int max_y = max(grid_1_y, grid_2_y);

                for (int k = min_x; k <= max_x; k++){
                    if (router.grid_graph[k][min_y].get_throughable() == 0) down_able = false;
                    if (router.grid_graph[k][max_y].get_throughable() == 0) up_able = false;
                }
                for (int k = min_y; k <= max_y; k++){
                    if (router.grid_graph[min_x][k].get_throughable() == 0) left_able = false;
                    if (router.grid_graph[max_x][k].get_throughable() == 0) right_able = false;
                }

                // if cant L-shape, then A-star
                if ((down_able == 1) && (right_able == 1)){

                }
                else if ((left_able == 1) && (up_able == 1)){

                }
                else{
                    // A*-search
                    Astar_out.push_back(astar_search(router, grid_2_x, grid_2_y, grid_1_x, grid_1_y));
                }
                */

                // A*-search
                if (!((grid_1_x == grid_2_x) && (grid_1_y == grid_2_y))){
                    vector<pair<int, int>> L_route = Z_shape(router, grid_1_x, grid_1_y, grid_2_x, grid_2_y);
                    if (!L_route.empty()) {
                        // for (auto coord: L_route) {
                        //     cout << coord.first << " " << coord.second << endl;
                        // }
                        Astar_out.push_back(L_route);
                    } else {
                        Astar_out.push_back(astar_search(router, grid_1_x, grid_1_y, grid_2_x, grid_2_y));
                    }
                }
                else{
                    Astar_out.push_back(vector<pair<int, int>>(2, make_pair(grid_1_x, grid_1_y)));
                }
            }
            else{
                //cout << "Impossible! there's no middle blks!" << endl;
            }
        }
        
        // store the result back to net-struct   
        //net[i]._Astar_out = Astar_out;
        outFile<<net[i].id<<endl;
        for (int j = 0; j < Astar_out.size(); j++){
            if(Astar_out[j].size() == 0) continue;
            for (int k = 0; k < Astar_out[j].size()-1; k++){
                outFile<<"("<<Astar_out[j][k].first<<","<<Astar_out[j][k].second<<"),("<<Astar_out[j][k+1].first<<","<<Astar_out[j][k+1].second<<")"<<endl;
            }
        }
        
        net[i]._Astar_out = Astar_out;
        //cerr << "net "<<i<<" finish, time = " << (chrono::duration_cast<std::chrono::milliseconds>(chrono::high_resolution_clock::now() - start).count()) / 1000.0 << endl;
    }
    cerr << "routing finish, time = " << (chrono::duration_cast<std::chrono::milliseconds>(chrono::high_resolution_clock::now() - start).count()) / 1000.0 << endl;

    // print wirenum
    // for (int i = 0; i < router.grid_index(router.getBoundingbox()).first+1; i++){
    //     for(int j = 0; j < router.grid_index(router.getBoundingbox()).second+1; j++){
    //         cout << router.grid_graph[i][j].get_wirenum() << " ";
    //     }
    // }
   
    // evaluator for overflow
    double overflow_cost = 0.0;
    // double cap_gcell_edge = maxTrack;
    for (int i = 0; i < net.size(); i++){
        double hpwl = net[i].CalHPWL();
        // double occupied_track = net.numTracks;
        vector<vector<pair<int, int>>> segmentList = net[i]._Astar_out;
        double segment_cost = 0.0;
        for (int j = 0; j < segmentList.size(); j++) {
            vector<pair<int, int>> twoPinSegment = segmentList[j];
            for (int k = 0; k < twoPinSegment.size(); k++) {
                Grid grid = router.grid_graph[twoPinSegment[k].first][twoPinSegment[k].second];
                double trackCost = grid.get_wirenum();
                // cout << "trackcost: " << trackCost << endl;
                segment_cost += trackCost;
            }
        }
        overflow_cost += (segment_cost / hpwl);
    }
    cout << "overflow_cost: " << overflow_cost << endl;
    cerr << "evaluator finish, time = " << (chrono::duration_cast<std::chrono::milliseconds>(chrono::high_resolution_clock::now() - start).count()) / 1000.0 << endl;

    
    return 0;
}

