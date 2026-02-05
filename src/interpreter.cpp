#include "interpreter.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <cctype>


using namespace std;


string getDirectory(const string& path) {
    size_t pos = path.find_last_of("/\\");
    return path.substr(0, pos + 1);
}

Interpreter::Interpreter(const string& programPath) 
    : lastReturned("NULL"), SPLAR('?'), pc(0)
{
    variables["EMPTY"] = "";
    variables["PC_IGNORE"] = "0";
    variables["ALE_VERSION"] = "v1.2";
    variables["ARG_PATH"] = getDirectory(programPath);
    variables["HELP_BMATH_OPERATORS"] = "+, -, *, /, root, powr";
    variables["HELP_IF_OPRS"] = "==, !=, >, <, >=, <=";
}


bool Interpreter::loadProgram(const string& filename) {
    ifstream file(filename);
    if (!file) return false;

    string line;
    while (getline(file, line)) {
        trim(line);
        programLines.push_back(line);
    }

    return true;
}

void Interpreter::run() {
    while (pc < (int)programLines.size()) {
        string line = programLines[pc++];
        if (line.empty() || line[0] == '#') continue;

        executeLine(line);
    }
}


vector<string> Interpreter::split(const string& line, char delimiter) {
    vector<string> result;
    result.reserve(8);
    size_t start = 0;
    size_t pos = line.find(delimiter, start);
    
    while (pos != string::npos) {
        string item = line.substr(start, pos - start);
        if (!item.empty() && item.front() != '"') {
            trim(item);
        }
        result.push_back(item);
        start = pos + 1;
        pos = line.find(delimiter, start);
    }
    
    string item = line.substr(start);
    if (!item.empty() && item.front() != '"') {
        trim(item);
    }
    result.push_back(item);
    
    return result;
}

void Interpreter::trim(string& str) {
    size_t start = 0;
    size_t end = str.length();
    while (start < end && isspace(str[start])) start++;
    while (end > start && isspace(str[end - 1])) end--;
    str = str.substr(start, end - start);
}