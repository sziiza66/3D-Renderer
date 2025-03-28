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

// Сократил название, иначе оно огромное.
struct PLSInSpace {
    PointLightSource source_data;
    Vector3 position = {0, 0, 0};
};

}  // namespace Renderer3D::Kernel
