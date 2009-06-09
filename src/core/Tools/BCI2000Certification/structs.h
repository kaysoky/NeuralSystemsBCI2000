#ifndef STRUCTS_H
#define STRUCTS_H
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
using namespace std;
class basicStats
{
public:
    double mean, std, min, max, median;
    vector<double> vals;
	vector<double> sigProc;
	vector<double> jitter;
	vector<double> stim;
	int size;
    string taskName;
    string desc;
};

class analysisType
{
public:
    int ch;
    std::string state;
    std::vector<int> stateVal;
    bool flag;
};
#endif