#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cassert>
#include <vector>
#include <cmath>
#include <map>
#include <algorithm>
#include "cell.h"
#include "net.h"
#include "partitioner.h"
using namespace std;


void Partitioner::parseInput(fstream& inFile)
{
    string str;
    // Set balance factor
    inFile >> str;
    _bFactor = stod(str);

    // Set up whole circuit
    while (inFile >> str) {
        if (str == "NET") {
            string netName, cellName, tmpCellName = "";
            inFile >> netName;
            int netId = _netNum;
            _netArray.push_back(new Net());
            while (inFile >> cellName) {
                if (cellName == ";") {
                    tmpCellName = "";
                    break;
                }
                else {
                    // a newly seen cell
                    if (_cellName2Id.count(cellName) == 0) {
                        int cellId = _cellNum;
                        _cellArray.push_back(new Cell(cellName, 0, cellId));
                        _cellName2Id[cellName] = cellId;
                        _cellArray[cellId]->addNet(netId);
                        _cellArray[cellId]->incPinNum();
                        _netArray[netId]->addCell(cellId);
                        ++_cellNum;
                        tmpCellName = cellName;
                    }
                    // an existed cell
                    else {
                        if (cellName != tmpCellName) {
                            assert(_cellName2Id.count(cellName) == 1);
                            int cellId = _cellName2Id[cellName];
                            _cellArray[cellId]->addNet(netId);
                            _cellArray[cellId]->incPinNum();
                            _netArray[netId]->addCell(cellId);
                            tmpCellName = cellName;
                        }
                    }
                }
            }
            ++_netNum;
        }
    }
    return;
}

void Partitioner::partition()
{
    //init partition
    for(int i = 0; i < _cellNum; i++){
        vector<int> netlist = _cellArray[i]->getNetList();
        _cellArray[i]->_sort_parameter = 0;
        for(int j = 0; j < netlist.size(); j++){
            _cellArray[i]->_sort_parameter = max(_cellArray[i]->_sort_parameter, int(_netArray[netlist[j]]->getCellList().size()));
        }
        _cellArray[i]->_sort_parameter = _cellArray[i]->_sort_parameter;
    }

    vector<Cell*> sorted_cell = _cellArray;
    sort(sorted_cell.begin(), sorted_cell.end(), [](Cell* a, Cell* b) {
        return (a->_sort_parameter > b->_sort_parameter);
    });

    //record max pinnum
    _maxPinNum = _cellArray.back()->getPinNum();
    for(int i = 0; i < _cellNum; i++){
        //put cell to a or b depending on part size
        vector<int> net_list = sorted_cell[i]->getNetList();
        if(_partSize[0] > _partSize[1]){
            for(int j = 0; j < net_list.size(); j++){
                _netArray[net_list[j]]->incPartCount(1);
            }
            sorted_cell[i]->move();
            _partSize[1]++;
        }
        else{
            for(int j = 0; j < net_list.size(); j++){
                _netArray[net_list[j]]->incPartCount(0);
            }
            _partSize[0]++;
        }
    }

    //FM
    bool part;
    while(1){
        //init
        //calculate gain
        for(int i = 0; i < _cellNum; i++){
            _cellArray[i]->setGain(0);
            vector<int> net_list = _cellArray[i]->getNetList();
            for(int j = 0; j < net_list.size(); j++){
                if(_netArray[net_list[j]]->getPartCount(_cellArray[i]->getPart()) == 1){
                    _cellArray[i]->incGain();
                }
                if(_netArray[net_list[j]]->getPartCount(!_cellArray[i]->getPart()) == 0){
                    _cellArray[i]->decGain();
                }
            }
        }
        //init bucket list
        for(int i = 0; i < _cellNum; i++){
            _cellArray[i]->unlock();
            _cellArray[i]->getNode()->setPrev(NULL);

            Node* target = _bList[_cellArray[i]->getPart()][_cellArray[i]->getGain()];
            _cellArray[i]->getNode()->setNext(target);
            _bList[_cellArray[i]->getPart()][_cellArray[i]->getGain()] = _cellArray[i]->getNode();

            if(target != NULL) target->setPrev(_cellArray[i]->getNode());
        }
        //init global variable
        _accCell.clear();
        _accGain.clear();
        _maxAccGain = 0;
        _AccGain = 0;

        //main algorithm
        while((_bList[0].size() > 0) && (_bList[1].size() > 0)){
            //choose max gain part
            if((_partSize[1]-1) < (_cellNum * (1.0-_bFactor) / 2) || ((_bList[0].begin()->first >= _bList[1].begin()->first) && (_partSize[0]-1) >= (_cellNum * (1.0-_bFactor) / 2)))
                part = 0;
            else
                part = 1;
        
            //move max gain
            _maxGainCell = _bList[part].begin()->second;
            if(_maxGainCell->getNext() != NULL){
                _maxGainCell->getNext()->setPrev(NULL);
                _bList[part].begin()->second = _maxGainCell->getNext();
                _maxGainCell->setNext(NULL);
            }
            else{
                _bList[part].erase(_cellArray[_maxGainCell->getId()]->getGain());
            }

            _partSize[part]--;
            _partSize[!part]++;
            _cellArray[_maxGainCell->getId()]->move();  
            _cellArray[_maxGainCell->getId()]->lock();  
            _accGain.push_back(_cellArray[_maxGainCell->getId()]->getGain());
            _accCell.push_back(_cellArray[_maxGainCell->getId()]);
            _AccGain += _accGain.back();
            _maxAccGain = (_AccGain > _maxAccGain)?_AccGain:_maxAccGain;

            //update
            update_gain(_cellArray[_maxGainCell->getId()], part);
        }

        part = (_bList[0].size() > 0)? 0:1;
        while((_bList[part].size() > 0) && ((_partSize[part]-1) >= (_cellNum * (1.0-_bFactor) / 2))){
            _maxGainCell = _bList[part].begin()->second;
            if(_maxGainCell->getNext() != NULL){
                _maxGainCell->getNext()->setPrev(NULL);
                _bList[part].begin()->second = _maxGainCell->getNext();
            }
            else{
                _bList[part].erase(_cellArray[_maxGainCell->getId()]->getGain());
            }
            _partSize[part]--;
            _partSize[!part]++;
            _cellArray[_maxGainCell->getId()]->move();  
            _cellArray[_maxGainCell->getId()]->lock();  
            _accGain.push_back(_cellArray[_maxGainCell->getId()]->getGain());
            _accCell.push_back(_cellArray[_maxGainCell->getId()]);
            _AccGain += _accGain.back();
            _maxAccGain = (_AccGain > _maxAccGain)?_AccGain:_maxAccGain;
            
            //update
            update_gain(_cellArray[_maxGainCell->getId()], part);
        }

        //find largest gain
        if(_maxAccGain <= 0){
            break;
        }
        else{
            while(_maxAccGain > _AccGain){

                _AccGain -= _accGain.back();
                _accGain.pop_back();

                Cell* target = _accCell.back();
                _accCell.pop_back();
                vector<int> netlist = target->getNetList();
                for(int i = 0; i < netlist.size(); i++){
                    _netArray[netlist[i]]->decPartCount(target->getPart());
                    _netArray[netlist[i]]->incPartCount(!target->getPart());
                }
                _partSize[target->getPart()]--;
                _partSize[!target->getPart()]++;
                target->move();
            }
        }
        
    }
    calculate_cut();
}

void Partitioner::update_gain(Cell* base_cell, bool from){
    bool to = !from;
    vector<int> net_list = base_cell->getNetList();
    for(int i = 0; i < net_list.size(); i++){
        //before move
        if(_netArray[net_list[i]]->getPartCount(to) == 0){
            vector<int> cell_list = _netArray[net_list[i]]->getCellList();
            for(int j = 0; j < cell_list.size(); j++){
                if(!_cellArray[cell_list[j]]->getLock()){
                    update_bucket(_cellArray[cell_list[j]], 1);
                    _cellArray[cell_list[j]]->incGain();
                }
            } 
        }
        else if(_netArray[net_list[i]]->getPartCount(to) == 1){
            vector<int> cell_list = _netArray[net_list[i]]->getCellList();
            for(int j = 0; j < cell_list.size(); j++){
                if((_cellArray[cell_list[j]]->getPart() == to) && (!_cellArray[cell_list[j]]->getLock())){
                    Cell* test_cell = _cellArray[cell_list[j]];
                    update_bucket(_cellArray[cell_list[j]], -1);
                    _cellArray[cell_list[j]]->decGain();
                    break;
                }
            }
        }

        //move
        _netArray[net_list[i]]->decPartCount(from);
        _netArray[net_list[i]]->incPartCount(to);

        //after move
        if(_netArray[net_list[i]]->getPartCount(from) == 0){
            vector<int> cell_list = _netArray[net_list[i]]->getCellList();
            for(int j = 0; j < cell_list.size(); j++){
                if(!_cellArray[cell_list[j]]->getLock()){
                    update_bucket(_cellArray[cell_list[j]], -1);
                    _cellArray[cell_list[j]]->decGain();
                }
            } 
        }
        else if(_netArray[net_list[i]]->getPartCount(from) == 1){
            vector<int> cell_list = _netArray[net_list[i]]->getCellList();
            for(int j = 0; j < cell_list.size(); j++){
                if((_cellArray[cell_list[j]]->getPart() == from) && (!_cellArray[cell_list[j]]->getLock())){
                    update_bucket(_cellArray[cell_list[j]], 1);
                    _cellArray[cell_list[j]]->incGain(); 
                    break;
                }
            }
        }
    }
}

void Partitioner::update_bucket(Cell* cell, int inc){
    //remove
    if((cell->getNode()->getNext() == NULL) && (cell->getNode()->getPrev() == NULL)){//only itself in bucket list
        _bList[cell->getPart()].erase(cell->getGain());
    }
    else if(cell->getNode()->getNext() == NULL){//tail
        cell->getNode()->getPrev()->setNext(NULL);
        cell->getNode()->setPrev(NULL);
    }
    else if(cell->getNode()->getPrev() == NULL){
        _bList[cell->getPart()][cell->getGain()] = cell->getNode()->getNext();
        cell->getNode()->getNext()->setPrev(NULL);
    }
    else{
        cell->getNode()->getNext()->setPrev(cell->getNode()->getPrev());
        cell->getNode()->getPrev()->setNext(cell->getNode()->getNext());
        cell->getNode()->setPrev(NULL);
    }

    //insert
    Node* target = _bList[cell->getPart()][cell->getGain()+inc];
    cell->getNode()->setNext(target);
    _bList[cell->getPart()][cell->getGain()+inc] = cell->getNode();
    if(target != NULL) target->setPrev(cell->getNode());
}

void Partitioner::calculate_cut(){
    _cutSize = 0;
    for(int i = 0; i < _netNum; i++){
        if((_netArray[i]->getPartCount(0) != 0) && (_netArray[i]->getPartCount(1) != 0)){
            _cutSize++;
        }
    }
}

void Partitioner::printSummary() const
{
    cout << endl;
    cout << "==================== Summary ====================" << endl;
    cout << " Cutsize: " << _cutSize << endl;
    cout << " Total cell number: " << _cellNum << endl;
    cout << " Total net number:  " << _netNum << endl;
    cout << " Cell Number of partition A: " << _partSize[0] << endl;
    cout << " Cell Number of partition B: " << _partSize[1] << endl;
    cout << "=================================================" << endl;
    cout << endl;
    return;
}

void Partitioner::writeResult(fstream& outFile)
{
    stringstream buff;
    buff << _cutSize;
    outFile << "Cutsize = " << buff.str() << '\n';
    buff.str("");
    buff << _partSize[0];
    outFile << "G1 " << buff.str() << '\n';
    for (size_t i = 0, end = _cellArray.size(); i < end; ++i) {
        if (_cellArray[i]->getPart() == 0) {
            outFile << _cellArray[i]->getName() << " ";
        }
    }
    outFile << ";\n";
    buff.str("");
    buff << _partSize[1];
    outFile << "G2 " << buff.str() << '\n';
    for (size_t i = 0, end = _cellArray.size(); i < end; ++i) {
        if (_cellArray[i]->getPart() == 1) {
            outFile << _cellArray[i]->getName() << " ";
        }
    }
    outFile << ";\n";
    return;
}

void Partitioner::clear()
{
    for (size_t i = 0, end = _cellArray.size(); i < end; ++i) {
        delete _cellArray[i];
    }
    for (size_t i = 0, end = _netArray.size(); i < end; ++i) {
        delete _netArray[i];
    }
    return;
}
