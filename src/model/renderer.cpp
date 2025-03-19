#include "renderer.h"

#include <cassert>
#include <vector>

namespace Renderer3D::Kernel {

namespace {

void ApplyFrustum(Triangle& triangle, const Camera& cam) {
    triangle.matrix = cam.GetFrustumMatrix() * triangle.matrix;
    for (uint8_t j = 0; j < 3; ++j) {
        for (uint8_t i = 0; i < 3; ++i) {
            triangle.matrix(j, i) = (triangle.matrix(3, i) != 0 ? triangle.matrix(j, i) / triangle.matrix(3, i) : 0);
        }
    }
}

void ClipWhen2BadPoints(std::vector<Triangle>& triangles, double near_dist, int8_t bad1, int8_t bad2,
                               int8_t good) {
    // Этот ассёрт не должен в теории никогда срабатывать, но пусть будет.
    assert(triangles.back().matrix(2, bad1) - triangles.back().matrix(2, good) != 0);

    triangles.back().matrix.col(bad1) =
        triangles.back().matrix.col(good) + (triangles.back().matrix.col(bad1) - triangles.back().matrix.col(good)) *
                                                (near_dist - triangles.back().matrix(2, good)) /
                                                (triangles.back().matrix(2, bad1) - triangles.back().matrix(2, good));
    // Этот ассёрт не должен в теории никогда срабатывать, но пусть будет.
    assert(triangles.back().matrix(2, bad2) - triangles.back().matrix(2, good) != 0);

    triangles.back().matrix.col(bad2) =
        triangles.back().matrix.col(good) + (triangles.back().matrix.col(bad2) - triangles.back().matrix.col(good)) *
                                                (near_dist - triangles.back().matrix(2, good)) /
                                                (triangles.back().matrix(2, bad2) - triangles.back().matrix(2, good));
    triangles.back().matrix(2, bad1) = near_dist;
    triangles.back().matrix(2, bad2) = near_dist;
}

void ClipWhen1BadPoint(std::vector<Triangle>& triangles, double near_dist, int8_t bad, int8_t good1,
                              int8_t good2) {
    // Этот ассёрт не должен в теории никогда срабатывать, но пусть будет.
    assert(triangles.back().matrix(2, bad) - triangles.back().matrix(2, good1) != 0);

    Vector4 v_bad2 = triangles.back().matrix.col(bad);

    triangles.back().matrix.col(bad) =
        triangles.back().matrix.col(good1) + (triangles.back().matrix.col(bad) - triangles.back().matrix.col(good1)) *
                                                 (near_dist - triangles.back().matrix(2, good1)) /
                                                 (triangles.back().matrix(2, bad) - triangles.back().matrix(2, good1));

    triangles.back().matrix(2, bad) = near_dist;

    triangles.emplace_back(triangles.back());
    // Этот ассёрт не должен в теории никогда срабатывать, но пусть будет.
    assert(v_bad2(2) - triangles.back().matrix(2, good2) != 0);

    triangles.back().matrix.col(good1) =
        triangles.back().matrix.col(good2) + (v_bad2 - triangles.back().matrix.col(good2)) *
                                                 (near_dist - triangles.back().matrix(2, good2)) /
                                                 (v_bad2(2) - triangles.back().matrix(2, good2));

    triangles.back().matrix(2, bad) = near_dist;
}

void ClipAgainstZAxis(std::vector<Triangle>& triangles, size_t ind, double near_dist) {
    int8_t bad1 = -1;
    int8_t bad2 = -1;
    int8_t good1 = -1;
    int8_t good2 = -1;
    for (uint8_t i = 0; i < 3; ++i) {
        if (triangles[ind].matrix(2, i) < near_dist) {
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

void FetchTriangles(std::vector<Triangle>& triangles, const std::vector<SubObject>& objects, size_t ind = 0) {
    for (const SubObject& sobj : objects) {
        for (const Triangle& triangle : sobj.obj.GetTriangles()) {
            triangles.emplace_back(triangle);
        }
        FetchTriangles(triangles, sobj.obj.GetSubobjects(), triangles.size());
        for (; ind < triangles.size(); ++ind) {
            triangles[ind].matrix = sobj.pos * triangles[ind].matrix;
        }
    }
}

}  // namespace

Frame Renderer::RenderFrame(const std::vector<SubObject>& objects, const Matrix4& camera_pos, const Camera& cam,
                            Frame&& frame) {

    triangle_buffer_.clear();
    FetchTriangles(triangle_buffer_, objects);
    Matrix4 cam_inverse = camera_pos.inverse();

    for (Triangle& triangle : triangle_buffer_) {
        triangle.matrix = cam_inverse * triangle.matrix;
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
