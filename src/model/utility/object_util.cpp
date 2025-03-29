#include "object_util.h"

namespace Renderer3D::Kernel {

Object CreateOctahedron(double scale, const Color& color) {
    // Тупенькая функция, возвращающая октаэдр, нормали настроены так, чтобы создавать иллюзию шара, 8 граней правда
    // мало, но зато просто.
    assert(!std::isinf(scale));
    assert(scale != 0);

    Object ret;
    for (uint8_t i = 0; i < 8; ++i) {
        Triangle t;
        t.diffuse_reflection_color = color;
        t.vertices(0, 0) = i % 2 ? scale : -scale;
        t.vertices(1, 1) = i % 4 < 2 ? scale : -scale;
        t.vertices(2, 2) = i % 8 < 4 ? scale : -scale;
        t.vertices(3, 0) = 1;
        t.vertices(3, 1) = 1;
        t.vertices(3, 2) = 1;
        t.vertex_normals.col(0) = t.vertices.col(0).head(3) / scale;
        t.vertex_normals.col(1) = t.vertices.col(1).head(3) / scale;
        t.vertex_normals.col(2) = t.vertices.col(2).head(3) / scale;
        ret.PushTriangle(t);
    }

    return ret;
}

}  // namespace Renderer3D::Kernel
