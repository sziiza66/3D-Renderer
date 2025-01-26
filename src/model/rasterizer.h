#pragma once
#include "frame.h"
#include "triangle.h"

namespace Renderer3D::Kernel {

class BufferRasterizer {
public:
    BufferRasterizer() = default;

    Frame MakeFrame(const std::vector<Triangle>& triangles, Frame&& frame);
};

}  // namespace Renderer3D::Kernel
