#pragma once

#include "frame.h"
#include "rasterizer.h"
#include "../model/camera.h"
#include "../model/object.h"

namespace Renderer3D::Kernel {

class Renderer {
    using Matrix4d = Eigen::Matrix4d;

public:
    Frame RenderFrame(const std::vector<SubObject>& objects, const Position& camera_pos, const Camera& cam,
                      size_t height, size_t width);

private:
    BufferRasterizer rasterizer_;
};

}  // namespace Renderer3D::Kernel
