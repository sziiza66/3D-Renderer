#pragma once

#include <tuple>

namespace Renderer3D {

std::tuple<char*, double> ParseCMD(int argc, char** argv);

void PrintReference();

}  // namespace Renderer3D
