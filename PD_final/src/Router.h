#ifndef ROUTER_H
#define ROUTER_H
#include <cassert>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <utility>
#include <fstream>
#include <json.hpp>
#include <DefParser.h>
#include <CfgJsonParser.h>
#include <ConnectionJsonParser.h>
#include <climits>
using namespace std;

using json = nlohmann::json;

//calcluate rectangle outline
class Polygon {
public:

    Polygon(vector<pair<int, int>> input) :
    vertices(input) { }

    void clear() {
        vertices.clear();
    }

    void addVertex(int x, int y) {
        vertices.push_back(make_pair(x, y));
    }

    void printVertices() const {
        for (const auto& vertex : vertices) {
            cout << "(" << vertex.first << ", " << vertex.second << ") ";
        }
        cout << endl;
    }

    bool isPointInside(int x, int y) const {
        int n = vertices.size();
        if (n < 3) return false;
        bool inside = false;


        for (int i = 0, j = n - 1; i < n; j = i++) {
            int xi = vertices[i].first, yi = vertices[i].second;
            int xj = vertices[j].first, yj = vertices[j].second;

            // 检查点是否在边上
            if (((yi > y) != (yj > y)) &&
                (x < (xj - xi) * (y - yi) / (yj - yi) + xi)) {
                inside = !inside;
            }

            // 如果点在多边形的边上，认为不在多边形内
            if ((y - yi) * (xj - xi) == (x - xi) * (yj - yi) &&
                min(xi, xj) <= x && x <= max(xi, xj) &&
                min(yi, yj) <= y && y <= max(yi, yj)) {
                return false;
            }
        }
        return inside;
    }

    void getBounds(int &minX, int &maxX, int &minY, int &maxY) const {
        minX = maxX = vertices[0].first;
        minY = maxY = vertices[0].second;

        for (const auto& vertex : vertices) {
            if (vertex.first < minX) minX = vertex.first;
            if (vertex.first > maxX) maxX = vertex.first;
            if (vertex.second < minY) minY = vertex.second;
            if (vertex.second > maxY) maxY = vertex.second;
        }
    }

private:
    vector<pair<int, int>> vertices;
};

class Grid{
    public:
        int color = 3;
        // constuctor
        Grid() { }
        Grid(int G, int cost, int x, int y, bool throughable) :
            _G(G), _cost(cost), _x(x), _y(y), _throughable(throughable), _prevgrid(nullptr), _wirenum(0), _explored(0) { }
        // get
        int get_G() {return _G;}
        int get_cost() {return _cost;}
        int get_x() {return _x;}
        int get_y() {return _y;}
        Grid* get_prev() { return _prevgrid;}
        bool get_throughable() { return _throughable;}
        vector<DefParser::Block*> get_blocks() { return _blocks;}
        int get_wirenum() {return _wirenum;}
        bool get_explored() { return _explored;}
        // set
        void set_G(int G) {_G = G;}
        void set_cost(int cost) {_cost = cost;}
        void set_x(int x) {_x = x;}
        void set_y(int y) {_y = y;}
        void set_prev(Grid* prev) {_prevgrid = prev;}
        void set_throughable(bool throughable) {_throughable = throughable;}
        void add_block(DefParser::Block* b) {_blocks.push_back(b);}
        void inc_wirenum() {_wirenum++;}
        void set_explored(bool explored) {_explored = explored;}

        // reset 
        void reset(){
            set_G(0);
            set_cost(0);
            set_explored(0);
        }
        
        // destructor
        ~Grid(){ }
    private:
        int _G;
        int _cost;
        int _x;
        int _y;
        Grid* _prevgrid;
        bool _throughable;
        vector<DefParser::Block*> _blocks;
        int _wirenum;
        bool _explored;
};

class Router {
private:
    int tracks_per_um;
    int maxTrack;
    pair<int, int> boundingbox;
    unordered_map<string, DefParser::Block> blocks;
    unordered_map<string, CfgJsonParser::BlockInfo> blockInfos;
    vector<ConnectionJsonParser::NetInfo> nets;
    double grid_width;

public:
    Router(int tpu, 
           int mT,
           const pair<int, int>& box,
           const unordered_map<string, DefParser::Block>& blks,
           const unordered_map<string, CfgJsonParser::BlockInfo>& blkInfos, 
           const vector<ConnectionJsonParser::NetInfo>& nts)
        : tracks_per_um(tpu), maxTrack(mT), boundingbox(box), blocks(blks), blockInfos(blkInfos), nets(nts) {
            //grid_width = ceil((double)maxTrack / (double)tracks_per_um);
            grid_width = 20000;

            cerr<<grid_index(getBoundingbox()).first+1<<" "<<grid_index(getBoundingbox()).second+1<<endl;
            //init blocks
            for (auto it = blocks.begin(); it != blocks.end(); ++it) {
                if(!it->second.region){
                    string blockName = "BLOCK_" + it->second.type.substr(it->second.type.find('_') + 1);
                    it->second.shape = blocks[blockName].shape;
                    it->second.throughBlockNetNum = blockInfos[it->second.name].throughBlockNetNum;
                    it->second.isFeedthroughable = blockInfos[it->second.name].isFeedthroughable;
                }
                else{
                    it->second.isFeedthroughable = true;
                }
            }

            //init grid_graph
            grid_graph = new Grid*[grid_index(boundingbox).first+1];
            for (int i = 0; i < grid_index(boundingbox).first+1; i++) {
                grid_graph[i] = new Grid[grid_index(boundingbox).second+1];
                for (int j = 0; j < grid_index(boundingbox).second+1; ++j) {
                    grid_graph[i][j] = Grid(0, 0, i, j, 1);
                }
            }

            //traverse blks and set throughable
            for (auto it = blocks.begin(); it != blocks.end(); ++it) {
                //calculate actual shape
                it->second.actual_shape = calculateActualCoordinate(it->second.position, it->second.shape, it->second.orientation);
                
                it->second.left_bottom = make_pair(INT_MAX, INT_MAX);

                //turn actual shape into grid coordinate
               for(int i = 0; i < it->second.actual_shape.size(); i++){
                    it->second.actual_shape[i] = grid_index(it->second.actual_shape[i]);
                }

                //set throughable
                Polygon polygon(it->second.actual_shape);
                int minX, maxX, minY, maxY;
                polygon.getBounds(minX, maxX, minY, maxY);
                for (int j = minY; j <= maxY; ++j) {
                    for (int i = minX; i <= maxX; ++i) {
                        assert(i < (grid_index(boundingbox).first+1));
                        assert(j < (grid_index(boundingbox).second+1));
                        if (polygon.isPointInside(i, j)) {
                            it->second.left_bottom.first = min(it->second.left_bottom.first, i);
                            it->second.left_bottom.second = min(it->second.left_bottom.second, j);
                            grid_graph[i][j].set_throughable(it->second.isFeedthroughable);
                            grid_graph[i][j].add_block(&it->second);
                        }
                    }
                }
                if(it->second.left_bottom.first == INT_MAX || it->second.left_bottom.second == INT_MAX){
                    it->second.left_bottom = it->second.position;
                }
                else{
                    it->second.left_bottom.first = it->second.left_bottom.first * grid_width;
                    it->second.left_bottom.second = it->second.left_bottom.second * grid_width;
                }
            }
        }
    
    //==================data member access==================
    Grid** grid_graph;

    // Function to get tracks per um
    const int& getTracksPerUm() const {
        return tracks_per_um;
    }

    // Function to get Boundingbox
    const pair<int, int>& getBoundingbox() const {
        return boundingbox;
    }

    // Function to access blocks
    const unordered_map<string, DefParser::Block>& getBlocks() const {
        return blocks;
    }

    // Function to access nets
    const vector<ConnectionJsonParser::NetInfo>& getNets() const {
        return nets;
    }

    //==================grid shift function==================
    // Function to calculate grid_index
    const pair<int, int> grid_index(pair<double, double> coordinate) const {
        int shift_x = floor(coordinate.first/grid_width);
        int shift_y = floor(coordinate.second/grid_width);
        return make_pair(shift_x, shift_y);
    }

    // Function to calculate grid_coordinate
    const pair<double, double> grid_coordinate(pair<double, double> coordinate) const {
        double shift_x = (floor(coordinate.first/grid_width)+0.5)*grid_width;
        double shift_y = (floor(coordinate.second/grid_width)+0.5)*grid_width;
        return make_pair(shift_x, shift_y);
    }

    //==================grid print function==================
    //used to output necessary information for visualization
    void printPolygon() {
        json j;
        int count = 0;
        for (auto it = blocks.begin(); it != blocks.end(); ++it) {
            vector<pair<int, int>> actualPoints = calculateActualCoordinate(it->second.position, it->second.shape, it->second.orientation);

            for(int i = 0; i < actualPoints.size(); i++){
                actualPoints[i] = grid_index(actualPoints[i]);
            }

            json poly;
            for (const auto& point : actualPoints) {
                poly.push_back({point.first, point.second});
            }
            if(it->second.region) j.push_back({{"points", poly}, {"color", "yellow"}});
            else if(it->second.isFeedthroughable) j.push_back({{"points", poly}, {"color", "orange"}});
            else j.push_back({{"points", poly}, {"color", "grey"}});

        }
        ofstream outFile("polygons.json");
        outFile << j.dump(4);
        outFile.close();
    }

    void printGrid() {
        for (auto it = blocks.begin(); it != blocks.end(); ++it) {
            //set throughable
            Polygon polygon(it->second.actual_shape);
            int minX, maxX, minY, maxY;
            polygon.getBounds(minX, maxX, minY, maxY);
            for (int i = minX; i <= maxX; ++i) {
                for (int j = minY; j <= maxY; ++j) {
                    assert(i < (grid_index(boundingbox).first+1));
                    assert(j < (grid_index(boundingbox).second+1));
                    if (polygon.isPointInside(i, j)) {
                        if(it->second.region) grid_graph[i][j].color = 2;
                        else if(it->second.isFeedthroughable) grid_graph[i][j].color = 1;
                        else grid_graph[i][j].color = 0;
                    }
                }
            }
        }

        ofstream outFile("grid.csv");
        for (int i = 0; i < grid_index(getBoundingbox()).first+1; ++i) {
            for (int j = 0; j < grid_index(getBoundingbox()).second+1; ++j) {
                outFile << grid_graph[i][j].color;
                if (j < grid_index(getBoundingbox()).second) {
                    outFile << ",";
                }
            }
            outFile << "\n";
        }
        outFile.close();
    }

    //==================other parse function==================
    pair<int, int> transformPoint(const pair<int, int>& point, const string& orientation, int width, int height) {
        if (orientation == "N") {
            // North:
            return point;
        } else if (orientation == "S") {
            // South:
            return pair<int, int>(width - point.first, height - point.second);
        } else if (orientation == "E") {
            // East:
            return pair<int, int>(height - point.second, point.first);
        } else if (orientation == "W") {
            // West:
            return pair<int, int>(point.second, width - point.first);
        } else if (orientation == "FN") {
            // Flipped North:
            return pair<int, int>(point.first, height - point.second);
        } else if (orientation == "FS") {
            // Flipped South:
            return pair<int, int>(width - point.first, point.second);
        } else if (orientation == "FW") {
            // Flipped West:
            return pair<int, int>(point.second, point.first);
        } else if (orientation == "FE") {
            // Flipped East:
            return pair<int, int>(height - point.second, width - point.first);
        } else {
            cerr << "Unknown orientation: " << orientation << endl;
            return pair<int, int>(0, 0);
        }
    }

    vector<pair<int, int>> calculateActualCoordinate(const pair<int, int>& origin, const vector<pair<int, int>>& relativePoints, const string& orientation) {
        //calculate width and height
        int minX = relativePoints[0].first, maxX = relativePoints[0].first;
        int minY = relativePoints[0].second, maxY = relativePoints[0].second;
        for (const auto& point : relativePoints) {
            if (point.first < minX) minX = point.first;
            if (point.first > maxX) maxX = point.first;
            if (point.second < minY) minY = point.second;
            if (point.second > maxY) maxY = point.second;
        }
        int width = maxX - minX;
        int height = maxY - minY;

        vector<pair<int, int>> actualPoints;
        for (const auto& relativePoint : relativePoints) {
            pair<int, int> transformedPoint = transformPoint(relativePoint, orientation, width, height);
            pair<int, int> actualPoint = make_pair(origin.first + transformedPoint.first, origin.second + transformedPoint.second);
            // cerr << "Relative Coordinate: (" << origin.first + transformedPoint.first << ", " << origin.second + transformedPoint.second << ")" << endl;
            // cerr << "Actual Coordinate: (" << actualPoint.first << ", " << actualPoint.second << ")" << endl;
            actualPoints.push_back(actualPoint);
        }
        return actualPoints;
    }

    // overflow evaluate
    double CalOverflowCost() {
        double cost = 0.0;
        // double cap_gcell_edge = maxTrack;
        for (int i = 0; i < nets.size(); i++){
            double hpwl = nets[i].CalHPWL();
            // double occupied_track = net.numTracks;
            vector<vector<pair<int, int>>> segmentList = nets[i]._Astar_out;
            double segment_cost = 0.0;
            for (int j = 0; j < segmentList.size(); j++) {
                vector<pair<int, int>> twoPinSegment = segmentList[j];
                for (int k = 0; k < twoPinSegment.size(); k++) {
                    Grid grid = grid_graph[twoPinSegment[k].first][twoPinSegment[k].second];
                    double trackCost = grid.get_wirenum();
                    cout << "trackcost: ";
                    cout << trackCost;
                    cout << endl;
                    segment_cost += trackCost;
                }
            }
            cost += (segment_cost / hpwl);
        }

        return cost;
    }
};


#endif // ROUTER_H