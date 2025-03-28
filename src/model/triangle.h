#pragma once

#include "color.h"
#include "../linalg.h"

namespace Renderer3D::Kernel {
namespace {

const TriMatrix kDefaultVertices{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {1, 1, 1}};

}

struct Triangle {
    TriMatrix vertices = kDefaultVertices;
    Matrix3 vertex_normals = Matrix3::Identity();
    Color diffuse_reflection_color;
};

}  // namespace Renderer3D::Kernel
