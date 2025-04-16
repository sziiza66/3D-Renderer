#pragma once

#include <fstream>
#include "../object.h"

namespace Renderer3D::Kernel {

Object ParseObj(std::ifstream& in, const Color& diffuse_reflection_color = {},
                const Color& specular_reflection_color = {}, uint32_t specular_power = 1, double scale = 1, std::string ptype = "");

}  // namespace Renderer3D::Kernel
