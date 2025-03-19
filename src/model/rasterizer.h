#pragma once
#include "frame.h"
#include "triangle.h"

namespace Renderer3D::Kernel {

class BufferRasterizer {
public:
    Frame MakeFrame(const std::vector<Triangle>& triangles, Frame&& frame);

private:
    std::vector<double> z_buffer_;
};

}  // namespace Renderer3D::Kernel
