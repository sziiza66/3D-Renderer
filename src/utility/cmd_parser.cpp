#include "cmd_parser.h"

#include <cmath>
#include <iostream>
#include <string>

namespace Renderer3D {

std::tuple<char*, double> ParseCMD(int argc, char** argv) {
    if (argc < 2) {
        return {nullptr, 0};
    }

    double scale = 1;
    if (argc > 2) {
        scale = std::stod(argv[2]);
    }
    if (std::isinf(scale) || std::isnan(scale)) {
        scale = 1;
    }

    return {argv[1], scale};
}

void PrintInfo() {
    std::cout
        << "Intended usage of the program:\n./renderer [path] [scale]\n[path] is the path to .obj file, [scale] is "
           "a real number.\nObject in [path] will be rendered [scale] times bigger than it is in the "
           "[path].\n[scale] is optional parameter and is equal to 1 by default.\n";
}

}  // namespace Renderer3D
