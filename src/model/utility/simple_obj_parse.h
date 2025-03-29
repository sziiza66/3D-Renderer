#pragma once

#include <fstream>
#include "../object.h"

namespace Renderer3D::Kernel {

Object ParseObj(std::ifstream& i, double scale = 1);

}  // namespace Renderer3D::Kernel
