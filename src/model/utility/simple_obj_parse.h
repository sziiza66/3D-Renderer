#pragma once

#include <fstream>
#include "../object.h"

namespace Renderer3D::Kernel {

Object ParseObj(std::ifstream& i, const Color& diffuse_reflection_color = {}, double scale = 1);

}  // namespace Renderer3D::Kernel
