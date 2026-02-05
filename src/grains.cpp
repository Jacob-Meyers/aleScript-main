#include "interpreter.h"
#include "grains.h"
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <chrono>
#include <string>
#include <sstream>
#include <cmath>

using namespace std;

// Command dispatcher table
static const unordered_map<string, function<void(Interpreter*, const vector<string>&)>> commandDispatcher = {
    {"print", [](Interpreter* self, const vector<string>& args) { self->executePrint(args); }},
    {"println", [](Interpreter* self, const vector<string>& args) { self->executePrintln(args); }},
    {"var", [](Interpreter* self, const vector<string>& args) { self->executeVar(args); }},
    {"bmath", [](Interpreter* self, const vector<string>& args) { self->executeBMath(args); }},
    {"input", [](Interpreter* self, const vector<string>& args) { self->executeInput(args); }},
    {"@REP_IGNORE", [](Interpreter* self, const vector<string>& args) { self->executeRep(false); }},
    {"@REP", [](Interpreter* self, const vector<string>& args) { self->executeRep(true); }},
    {"@SET_SPLAR", [](Interpreter* self, const vector<string>& args) { self->executeSET_SPLAR(args); }},
    {"fwrite", [](Interpreter* self, const vector<string>& args) { self->executeFWrite(args); }},
    {"join", [](Interpreter* self, const vector<string>& args) { self->executeJoin(args); }},
    {"system", [](Interpreter* self, const vector<string>& args) { self->executeSystem(args); }},
    {"if", [](Interpreter* self, const vector<string>& args) { self->executeIfStatement(args); }},
    {"round", [](Interpreter* self, const vector<string>& args) { self->executeRounding(args); }},
    {"webget", [](Interpreter* self, const vector<string>& args) { self->executeWebGet(args); }},
    {"getFps", [](Interpreter* self, const vector<string>& args) { self->executeGetFPS(); }},
    {"jump", [](Interpreter* self, const vector<string>& args) { self->executeJumpLines(args); }},
    {"exit", [](Interpreter* self, const vector<string>& args) { exit(0); }},
};

void Interpreter::executeLine(const string& line) {
    variables["LAST_RETURNED"] = lastReturned;

    auto csline = split(line, SPLAR);
    if (!pcLookingFor.empty() && pcLookingFor!=line) csline.clear();
    if (csline.empty() || csline[0].rfind("//", 0) == 0 || csline[0].rfind("-<", 0) == 0) return;

    auto it = commandDispatcher.find(csline[0]);
    if (it != commandDispatcher.end()) {
        it->second(this, csline);
        return;
    }

    auto extraIt = extraGrains.find(csline[0]);
    if (extraIt != extraGrains.end()) {
        extraIt->second(csline);
        return;
    }

    // Unknown command
    cerr << "Line " << pc << " ; Unknown grain > " << csline[0] << endl;
}

// println?<"value/var>
void Interpreter::executePrint(const vector<string>& csline) {
    if (csline.size() < 2) {
        cerr << "Line " << pc << " ; Invalid 'print' format" << endl;
        exit(0);
    }
    cout << getValue(csline[1]);
}

// println?<"value/var>
void Interpreter::executePrintln(const vector<string>& csline) {
    if (csline.size() < 2) {
        cerr << "Line " << pc << " ; Invalid 'println' format" << endl;
        exit(0);
    }
    cout << getValue(csline[1]) << endl;
}

// var?<var name>?<var value>   ---   ?<var value> <-- Optional
void Interpreter::executeVar(const vector<string>& csline) {
    if (csline.size() < 2) { cerr << "Line " << pc << " ; Invalid \'var\' format" << endl; exit(0); }

    variables[csline[1]] = (csline.size() == 2) ? lastReturned : getValue(csline[2]);
}

// bmath?<num1>?<operator>?<num2>
void Interpreter::executeBMath(const vector<string>& csline) {
    if (csline.size() >= 4) {
        float in1 = strtof(getValue(csline[1]).c_str(), nullptr);
        string sign = getValue(csline[2]);
        float in2 = strtof(getValue(csline[3]).c_str(), nullptr);
        float out = numeric_limits<float>::quiet_NaN();

        if (sign == "+") out = in1 + in2;
        else if (sign == "-") out = in1 - in2;
        else if (sign == "*") out = in1 * in2;
        else if (sign == "/") out = in1 / in2;
        else if (sign == "root") out = pow(in1, 1.0 / in2);
        else if (sign == "powr") out = pow(in1, in2);

        ostringstream oss;
        if (!isnan(out)) oss << out;
        else oss << "INVALID_OPERATOR";
        lastReturned = oss.str();
    } else {
        cerr << "Line " << pc << " ; Invalid 'bmath' format." << endl;
        lastReturned = "BMATH_ERROR";
    }
}

// input?<texttoput>
void Interpreter::executeInput(const vector<string>& csline) {
    if (csline.size() < 2) { cerr << "Line " << pc << " ; Invalid \'input\' format." << endl; exit(0); }
    cout << getValue(csline[1]);
    getline(cin, lastReturned);
    if (lastReturned.empty()) {
        lastReturned = "";
    }
}

// @REP ; @REP_IGNORE
void Interpreter::executeRep(bool type) {
    if (type) { // @REP
        pc = stoi(variables["PC_IGNORE"]);
    } else {    // @REP_IGNORE
        variables["PC_IGNORE"] = to_string(pc);
    }
}


// fwrite?<filepath>?<mode>?<contents>
void Interpreter::executeFWrite(const vector<string>& csline) {
    if (csline.size() < 3) { cerr << "Line " << pc << " ; Invalid \'fwrite\' format." << endl; exit(0); }

    string filepath = getValue(csline[1]);
    string mode = getValue(csline[2]);
    string fileContents = csline.size() > 3 ? getValue(csline[3]) : "";


    if (mode == "w") {
        ofstream(filepath, ios::out | ios::trunc) << fileContents << endl;
        lastReturned = "FWRITE_WRITE_TO_FILE";
    } 
    else if (mode == "a") {
        ofstream(filepath, ios::out | ios::app) << fileContents << endl;
        lastReturned = "FWRITE_APPEND_TO_FILE";
    } 
    else if (mode == "r") {
        ifstream inFile(filepath);
        if (inFile) lastReturned = string((istreambuf_iterator<char>(inFile)), istreambuf_iterator<char>());
    }
}

// join?<var1>?<var2>? --> <more vars>?
void Interpreter::executeJoin(const vector<string>& csline) {
    if (csline.size() < 2) {
        cerr << "Line " << pc << " ; Invalid 'join' format." << endl;
        return;
    }

    lastReturned.clear();
    for (size_t i = 1; i < csline.size(); ++i)
        lastReturned += !csline[i].empty() && csline[i][0] == '"' ? csline[i].substr(1) : variables[csline[i]];
}

// @SET_SPLAR?<new SPLAR>
void Interpreter::executeSET_SPLAR(const vector<string>& csline) {
    if (csline.size() < 2 || csline[1].empty()) cerr << "@SET_SPLAR requires a character argument" << endl;
    SPLAR = this->getValue(csline[1])[0];
}

// system?<command>
void Interpreter::executeSystem(const vector<string>& csline) {
    if (csline.size() < 2 || csline[1].empty()) cerr << "Line " << pc << " ; Invalid \'system\' format." << endl;
    system(getValue(csline[1]).c_str());
}

// systemsil?<command>
void Interpreter::executeSystemsil(const vector<string>& csline) {
    string cmd = getValue(csline[1]);
    #ifdef _WIN32
        cmd += " > NUL 2>&1";
    #else
        cmd += " > /dev/null 2>&1";
    #endif
    system(cmd.c_str());
}


// if?<in1>?<opr>?<in2>?<lines to move if false>
void Interpreter::executeIfStatement(const vector<string>& csline) {
    if (csline.size() < 2 || csline[1].empty()) cerr << "Line " << pc << " ; Invalid \'system\' format." << endl;

    string in1 = getValue(csline[1]);
    string in2 = getValue(csline[3]);
    string opr = getValue(csline[2]);
    int linesTM = stoi(getValue(csline[4]));
    string until = "-<"+getValue(csline[5]);
    bool out = false;

    if (opr == "==") out = (in1 == in2);
    else if (opr == "!=") out = (in1 != in2);
    else if (opr == ">") out = (stof(in1) > stof(in2));
    else if (opr == "<") out = (stof(in1) < stof(in2));
    else if (opr == ">=") out = (stof(in1) >= stof(in2));
    else if (opr == "<=") out = (stof(in1) <= stof(in2));
    else cerr << "Line " << pc << " ; Invalid \'if\' operator." << endl;
    if (!out) {
        if (!until.empty()) pcLookingFor = until;
        else pc += linesTM;
    }

    lastReturned = out ? "true" : "false";
}

// round?<num>?<floor/ceil>
void Interpreter::executeRounding(const vector<string>& csline) {
    if (csline.size() < 3 || csline[1].empty()) cerr << "Line " << pc << " ; Invalid \'round\' format." << endl;

    float num = stof(getValue(csline[1]));
    string type = getValue(csline[2]);

    if (type == "floor") lastReturned = to_string((int)floor(num));
    else if (type == "ceil") lastReturned = to_string((int)ceil(num));
    else lastReturned = "INVALID_ROUND_TYPE";
}

// webget?<url>?<path to save>
void Interpreter::executeWebGet(const vector<string>& csline) {
    string url = getValue(csline[1]);
    string output = getValue(csline[2]);

    #ifdef _WIN32
        string command = "curl -s -o \"" + output + "\" \"" + url + "\"";
    #else
        string command = "wget -q -O \"" + output + "\" \"" + url + "\"";
    #endif

    system(command.c_str());
}


void Interpreter::executeGetFPS() {
    using namespace chrono;

    fpsFrames++;
    auto now = high_resolution_clock::now();
    duration<float> elapsed = now - fpsLastTime;

    if (elapsed.count() >= 0.01f) {
        lastFPS = fpsFrames / elapsed.count();
        fpsFrames = 0;
        fpsLastTime = now;
    }

    lastReturned = to_string(lastFPS);
}

void Interpreter::executeJumpLines(const vector<string>& csline) {
    pc += stoi(getValue(csline[0]));
}