#pragma once
#include "frame.h"
#include "../model/entity.h"
#include "../model/triangle.h"

namespace Renderer3D::Kernel {

class BufferRasterizer {
public:
    BufferRasterizer() = default;

    Frame MakeFrame(size_t height, size_t width, const std::vector<std::pair<entity::Property, Triangle>>& triangles);
};

}  // namespace Renderer3D::Kernel
