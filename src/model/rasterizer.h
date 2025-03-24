#pragma once
#include "frame.h"
#include "triangle.h"
#include "zbuffer.h"

namespace Renderer3D::Kernel {

class BufferRasterizer {
public:
    Frame MakeFrame(const std::vector<Triangle>& triangles, Frame&& frame);

private:
    ZBuffer z_buffer_;
};

}  // namespace Renderer3D::Kernel
