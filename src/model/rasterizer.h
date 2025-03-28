#pragma once

#include "light.h"
#include "triangle.h"
#include "zbuffer.h"

namespace Renderer3D::Kernel {

class BufferRasterizer {
public:
    Frame MakeFrame(const std::vector<Triangle>& triangles, const std::vector<TriMatrix>& preserved,
                    const std::vector<PLSInSpace>& pls, const std::vector<Vector3>& normals, const Color& ambient,
                    Frame&& frame);

private:
    ZBuffer z_buffer_;
};

}  // namespace Renderer3D::Kernel
