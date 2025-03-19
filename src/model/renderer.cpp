#include "renderer.h"

#include <cassert>
#include <vector>

namespace Renderer3D::Kernel {

namespace {

void ApplyFrustum(Triangle& triangle, const Camera& cam) {
    triangle.vertices = cam.GetFrustumMatrix() * triangle.vertices;
    for (uint8_t j = 0; j < 3; ++j) {
        for (uint8_t i = 0; i < 3; ++i) {
            triangle.vertices(j, i) =
                (triangle.vertices(3, i) != 0 ? triangle.vertices(j, i) / triangle.vertices(3, i) : 0);
        }
    }
}

void ClipWhen2BadPoints(std::vector<Triangle>& triangles, double near_dist, int8_t bad1, int8_t bad2, int8_t good) {
    // Этот ассёрт не должен в теории никогда срабатывать, но пусть будет.
    assert(triangles.back().vertices(2, bad1) - triangles.back().vertices(2, good) != 0);

    triangles.back().vertices.col(bad1) = triangles.back().vertices.col(good) +
                                          (triangles.back().vertices.col(bad1) - triangles.back().vertices.col(good)) *
                                              (near_dist - triangles.back().vertices(2, good)) /
                                              (triangles.back().vertices(2, bad1) - triangles.back().vertices(2, good));
    // Этот ассёрт не должен в теории никогда срабатывать, но пусть будет.
    assert(triangles.back().vertices(2, bad2) - triangles.back().vertices(2, good) != 0);

    triangles.back().vertices.col(bad2) = triangles.back().vertices.col(good) +
                                          (triangles.back().vertices.col(bad2) - triangles.back().vertices.col(good)) *
                                              (near_dist - triangles.back().vertices(2, good)) /
                                              (triangles.back().vertices(2, bad2) - triangles.back().vertices(2, good));
    triangles.back().vertices(2, bad1) = near_dist;
    triangles.back().vertices(2, bad2) = near_dist;
}

void ClipWhen1BadPoint(std::vector<Triangle>& triangles, double near_dist, int8_t bad, int8_t good1, int8_t good2) {
    // Этот ассёрт не должен в теории никогда срабатывать, но пусть будет.
    assert(triangles.back().vertices(2, bad) - triangles.back().vertices(2, good1) != 0);

    Vector4 v_bad2 = triangles.back().vertices.col(bad);

    triangles.back().vertices.col(bad) = triangles.back().vertices.col(good1) +
                                         (triangles.back().vertices.col(bad) - triangles.back().vertices.col(good1)) *
                                             (near_dist - triangles.back().vertices(2, good1)) /
                                             (triangles.back().vertices(2, bad) - triangles.back().vertices(2, good1));

    triangles.back().vertices(2, bad) = near_dist;

    triangles.emplace_back(triangles.back());
    // Этот ассёрт не должен в теории никогда срабатывать, но пусть будет.
    assert(v_bad2(2) - triangles.back().vertices(2, good2) != 0);

    triangles.back().vertices.col(good1) =
        triangles.back().vertices.col(good2) + (v_bad2 - triangles.back().vertices.col(good2)) *
                                                   (near_dist - triangles.back().vertices(2, good2)) /
                                                   (v_bad2(2) - triangles.back().vertices(2, good2));

    triangles.back().vertices(2, bad) = near_dist;
}

void ClipAgainstZAxis(std::vector<Triangle>& triangles, size_t ind, double near_dist) {
    int8_t bad1 = -1;
    int8_t bad2 = -1;
    int8_t good1 = -1;
    int8_t good2 = -1;
    for (uint8_t i = 0; i < 3; ++i) {
        if (triangles[ind].vertices(2, i) < near_dist) {
            bad1 = i;
            std::swap(bad1, bad2);
        } else {
            good1 = i;
            std::swap(good1, good2);
        }
    }
    // Весь треугольник выкидывается.
    if (good2 == -1) {
        std::swap(triangles[ind], triangles.back());
        triangles.pop_back();
        return;
    }
    // Клиппинг не нужен.
    if (bad2 == -1) {
        return;
    }
    std::swap(triangles[ind], triangles.back());
    // Случай 1
    if (bad1 != -1 && bad2 != -1) {
        ClipWhen2BadPoints(triangles, near_dist, bad1, bad2, good2);
        return;
    }
    // Случай 2
    ClipWhen1BadPoint(triangles, near_dist, bad2, good1, good2);
}

void FetchTriangles(const std::vector<SubObject>& objects, std::vector<Triangle>& triangles, size_t ind = 0) {
    for (const SubObject& sobj : objects) {
        for (const Triangle& triangle : sobj.obj.GetTriangles()) {
            triangles.emplace_back(triangle);
        }
        FetchTriangles(sobj.obj.GetSubobjects(), triangles, triangles.size());
        for (; ind < triangles.size(); ++ind) {
            triangles[ind].vertices = sobj.pos * triangles[ind].vertices;
        }
    }
}

}  // namespace

Frame Renderer::RenderFrame(const std::vector<SubObject>& objects, const HomoTransform& camera_pos, const Camera& cam,
                            Frame&& frame) {
    triangle_buffer_.clear();
    FetchTriangles(objects, triangle_buffer_);
    HomoTransform cam_inverse = camera_pos.inverse();

    for (Triangle& triangle : triangle_buffer_) {
        triangle.vertices = cam_inverse * triangle.vertices;
    }
    // Clipping
    for (size_t i = triangle_buffer_.size() - 1; ~i; --i) {
        ClipAgainstZAxis(triangle_buffer_, i, cam.GetNearDist());
    }
    for (Triangle& triangle : triangle_buffer_) {
        ApplyFrustum(triangle, cam);
    }

    return rasterizer_.MakeFrame(triangle_buffer_, std::move(frame));
}

}  // namespace Renderer3D::Kernel
