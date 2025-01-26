#include "renderer.h"

#include <vector>

namespace Renderer3D::Kernel {

namespace {

void FetchTriangles(std::vector<Triangle>* triangles, const std::vector<SubObject>& objects, size_t ind = 0) {
    for (const SubObject& sobj : objects) {
        for (const Triangle& triangle : sobj.obj.GetTriangles()) {
            triangles->emplace_back(triangle);
        }
        FetchTriangles(triangles, sobj.obj.GetSubobjects(), triangles->size());
        for (; ind < triangles->size(); ++ind) {
            (*triangles)[ind].ApplyPosition(sobj.pos);
        }
    }
}

}  // namespace

Frame Renderer::RenderFrame(const std::vector<SubObject>& objects, const Position& camera_pos, const Camera& cam,
                            Frame&& frame) {

    std::vector<Triangle> triangles;
    FetchTriangles(&triangles, objects);
    Position cam_inverse = camera_pos.Inverse();

    for (Triangle& triangle : triangles) {
        triangle.ApplyPosition(cam_inverse);
        triangle.ApplyFrustrum(cam);
    }

    return rasterizer_.MakeFrame(triangles, std::move(frame));
}

}  // namespace Renderer3D::Kernel
