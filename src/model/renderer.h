#pragma once

#include "rasterizer.h"
#include "camera.h"
#include "object.h"

namespace Renderer3D::Kernel {

class Renderer {
public:
    Frame RenderFrame(const std::vector<SubObject>& objects, const AffineTransform& camera_pos, const Camera& cam,
                      const Color& ambient_light, std::vector<DirectionalLightSource> dir_light_buffer, Frame&& frame);

private:
    void ClearAllBuffers();

private:
    BufferRasterizer rasterizer_;
    std::vector<Triangle> triangle_buffer_;
    std::vector<PLSInSpace> point_light_buffer_;
    std::vector<SLSInSpace> spot_light_buffer_;
};

}  // namespace Renderer3D::Kernel
