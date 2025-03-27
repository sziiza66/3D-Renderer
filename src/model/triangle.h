#pragma once

#include "color.h"
#include "../linalg.h"

namespace Renderer3D::Kernel {

struct Triangle {
    TriMatrix vertices;
    Matrix3 vertex_normals;
    Color color;
};

}  // namespace Renderer3D::Kernel
