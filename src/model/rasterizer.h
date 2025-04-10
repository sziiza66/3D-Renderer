#pragma once

#include "camera.h"
#include "light.h"
#include "triangle.h"
#include "zbuffer.h"

namespace Renderer3D::Kernel {

// Сократил название, иначе оно огромное.
// Эта структура не испльзуется нигде, кроме растеризации и рендеринга, поэтому я решил её оставить тут.
struct PLSInSpace {
    PointLightSource source_data;
    Vector3 position = {0, 0, 0};
};

struct SLSInSpace {
    SpotLightSource source_data;
    Vector3 position = {0, 0, 0};
};

class BufferRasterizer {
public:
    Frame MakeFrame(const std::vector<Triangle>& triangles, const std::vector<PLSInSpace>& point_lights,
                    const std::vector<SLSInSpace>& spot_lights, const Color& ambient,
                    const std::vector<DirectionalLightSource>& directional_lights, const Camera& camera, Frame&& frame);

private:
    // Думаю, стоит добавить также буфер нормалей и точек, таким образом вычисление цвета пикселя будет проиходить не
    // больше чем Height*Width раз за кадр, если треугльников много, это должно быть значительным.
    ZBuffer z_buffer_;
};

}  // namespace Renderer3D::Kernel
