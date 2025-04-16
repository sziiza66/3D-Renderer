#pragma once

#include <tuple>

namespace Renderer3D {

std::tuple<char*, double> ParseCMD(int argc, char** argv);

void PrintInfo();

}  // namespace Renderer3D
