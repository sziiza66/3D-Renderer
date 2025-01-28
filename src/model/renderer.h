#pragma once

#include "frame.h"
#include "rasterizer.h"
#include "camera.h"
#include "object.h"

namespace Renderer3D::Kernel {

class Renderer {
    using Matrix4d = Eigen::Matrix4d;

public:
    Frame RenderFrame(const std::vector<SubObject>& objects, const Position& camera_pos, const Camera& cam,
                      Frame&& frame);

private:
    BufferRasterizer rasterizer_;
    std::vector<Triangle> triangle_buffer_;
};

}  // namespace Renderer3D::Kernel
