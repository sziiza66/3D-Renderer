#pragma once

#include "color.h"
#include "../aliases.h"

namespace Renderer3D::Kernel {

struct Triangle {
    TriMatrix vertices;
    Color color;
};

}  // namespace Renderer3D::Kernel
