#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <climits>
#include <json.hpp>
#include <climits>

using namespace std;

class Grid;

class ConnectionJsonParser {
public:
    struct NetInfo {
        int id;
        string tx;
        vector<string> rx;
        int numTracks;
        pair<double, double> txCoord;
        vector<pair<double, double>> rxCoord;
        // sun
        // result
        vector<vector<pair<int, int>>> _Astar_out;
        double CalHPWL() {
            double min_x = -INT_MAX;
            double max_x = INT_MAX;
            double min_y = -INT_MAX;
            double max_y = INT_MAX;
            for (int i = 0; i < _Astar_out.size(); i++) {
                vector<pair<int, int>> twoPinSegment = _Astar_out[i];
                for (int j = 0; j < twoPinSegment.size(); j++) {
                    double x = twoPinSegment[j].first;
                    double y = twoPinSegment[j].second;
                    min_x = min(min_x, x);
                    max_x = max(max_x, x);
                    min_y = min(min_y, y);
                    max_y = max(max_y, y);
                }
            }
            return (max_x - min_x) + (max_y - min_y);
        }

        NetInfo(int id, const string& tx, const vector<string>& rx, int num,
                pair<double, double> txc, const vector<pair<double, double>>& rxc)
            : id(id), tx(tx), rx(rx), numTracks(num), txCoord(txc), rxCoord(rxc) {}
    };

private:
    string filePath;
    vector<NetInfo> nets;
    int maxTrack = 0;

public:
    ConnectionJsonParser(const string& path) : filePath(path) {}

    bool parse() {
        ifstream jsonFile(filePath);
        if (!jsonFile.is_open()) {
            cerr << "Error opening file: " << filePath << endl;
            return false;
        }

        nlohmann::json j;
        jsonFile >> j;

        for (const auto& net : j) {
            if(net["NUM"] > maxTrack) {
                maxTrack = net["NUM"];
            }
            vector<string> rx = net["RX"];
            vector<pair<double, double>> rxCoord;
            for (const auto& coord : net["RX_COORD"]) {
                rxCoord.emplace_back(coord[0], coord[1]);
            }
            NetInfo ni(
                net["ID"],
                net["TX"],
                rx,
                net["NUM"],
                {net["TX_COORD"][0], net["TX_COORD"][1]},
                rxCoord
            );
            nets.push_back(ni);
        }

        jsonFile.close();
        return true;
    }

    const vector<NetInfo>& getNets() const {
        return nets;
    }

    const int& getmaxTrack() const {
        return maxTrack;
    }
};
