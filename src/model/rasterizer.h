#pragma once

#include "camera.h"
#include "light.h"
#include "triangle.h"
#include "zbuffer.h"

namespace Renderer3D::Kernel {

class BufferRasterizer {
public:
    Frame MakeFrame(const std::vector<Triangle>& triangles, const std::vector<PLSInSpace>& pls, const Color& ambient,
                    const Camera& camera, Frame&& frame);

private:
    ZBuffer z_buffer_;
};

}  // namespace Renderer3D::Kernel
