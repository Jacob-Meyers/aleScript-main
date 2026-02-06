#include "interpreter.h"

int main(int argc, char* argv[]) {
    string ale_version = "v1.3-n2";

    if (argc < 2) {
        std::cerr << "Alescript " << ale_version << "\nUsage: interpreter <program_file>\n";
        return 1;
    }

    Interpreter interpreter(argv[1]);
    if (!interpreter.loadProgram(argv[1])) {
        std::cerr << "Could not load program: " << argv[1] << "\n";
        return 1;
    }

    interpreter.run();
    return 0;
}
