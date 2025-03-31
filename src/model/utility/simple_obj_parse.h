#pragma once

#include <fstream>
#include "../object.h"

namespace Renderer3D::Kernel {

Object ParseObj(std::ifstream& i, double correction_eps = 0, const Color& diffuse_reflection_color = {},
                const Color& specular_reflection_color = {}, uint32_t specular_power = 1, double scale = 1);

}  // namespace Renderer3D::Kernel
