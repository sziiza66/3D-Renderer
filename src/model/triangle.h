#pragma once

#include "color.h"
#include "../linalg.h"

namespace Renderer3D::Kernel {

struct Triangle {
    TriMatrix vertices;
    Color color;
};

}  // namespace Renderer3D::Kernel
