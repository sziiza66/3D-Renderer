#pragma once

#include "color.h"
#include "../linalg.h"

namespace Renderer3D::Kernel {

struct PointLightSource {
    Color color;
    double k_const = 1;
    double k_linear = 0;
    double k_quadr = 0;
};

struct DirectionalLightSource {
    Color color;
    Vector3 direction;
};

struct SpotLightSource {
    Color color;
    Vector3 direction;
    double k_const = 1;
    double k_linear = 0;
    double k_quadr = 0;
    uint32_t concentration = 1;
};

}  // namespace Renderer3D::Kernel
