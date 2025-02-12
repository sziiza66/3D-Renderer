#include "renderer.h"

#include <vector>

namespace Renderer3D::Kernel {

namespace {

void ApplyFrustum(Triangle& triangle, const Camera& cam) {
    triangle.matrix = cam.GetFrustum() * triangle.matrix;
    for (uint8_t j = 0; j < 3; ++j) {
        for (uint8_t i = 0; i < 3; ++i) {
            triangle.matrix(j, i) /= triangle.matrix(3, i);
        }
    }
}

void FetchTriangles(std::vector<Triangle>* triangles, const std::vector<SubObject>& objects, size_t ind = 0) {
    for (const SubObject& sobj : objects) {
        for (const Triangle& triangle : sobj.obj.GetTriangles()) {
            triangles->emplace_back(triangle);
        }
        FetchTriangles(triangles, sobj.obj.GetSubobjects(), triangles->size());
        for (; ind < triangles->size(); ++ind) {
            (*triangles)[ind].matrix = sobj.pos * (*triangles)[ind].matrix;
        }
    }
}

}  // namespace

Frame Renderer::RenderFrame(const std::vector<SubObject>& objects, const Position& camera_pos, const Camera& cam,
                            Frame&& frame) {

    triangle_buffer_.clear();
    FetchTriangles(&triangle_buffer_, objects);
    Position cam_inverse = camera_pos.inverse();

    for (Triangle& triangle : triangle_buffer_) {
        triangle.matrix = cam_inverse * triangle.matrix;
        ApplyFrustum(triangle, cam);
    }

    return rasterizer_.MakeFrame(triangle_buffer_, std::move(frame));
}

}  // namespace Renderer3D::Kernel
