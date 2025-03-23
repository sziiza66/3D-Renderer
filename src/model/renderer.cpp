#include "renderer.h"

#include <cassert>
#include <vector>

namespace Renderer3D::Kernel {

namespace {

void ApplyFrustumTransformationOnTriangle(const Camera& cam, Triangle* triangle) {
    assert(triangle);
    triangle->vertices = cam.GetFrustumMatrix() * triangle->vertices;

    // Это лучшее, что можно сделать с помощью Eigen, библиотека не поддерживает column-wise применение кастомных
    // функций. Круто, если это может скомпилироваться inplace при O3 мб, но я че-то сомневаюсь.
    triangle->vertices = triangle->vertices.colwise().hnormalized().colwise().homogeneous();
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

void ClipAgainstZAxis(double near_dist, std::vector<Triangle>* triangles) {
    for (auto triangle : *triangles) {
        int8_t bad1 = -1;
        int8_t bad2 = -1;
        int8_t good1 = -1;
        int8_t good2 = -1;
        for (uint8_t i = 0; i < 3; ++i) {
            if (triangle.vertices(2, i) < near_dist) {
                bad1 = i;
                std::swap(bad1, bad2);
            } else {
                good1 = i;
                std::swap(good1, good2);
            }
        }
        // Весь треугольник выкидывается.
        if (good2 == -1) {
            std::swap(triangle, triangles->back());
            triangles->pop_back();
            return;
        }
        // Клиппинг не нужен.
        if (bad2 == -1) {
            return;
        }
        std::swap(triangle, triangles->back());
        // Случай 1
        if (bad1 != -1 && bad2 != -1) {
            ClipWhen2BadPoints(*triangles, near_dist, bad1, bad2, good2);
            return;
        }
        // Случай 2
        ClipWhen1BadPoint(*triangles, near_dist, bad2, good1, good2);
    }
}

void FetchTriangles(const SubObject& obj_with_pos, std::vector<Triangle>* triangles) {
    assert(triangles);
    size_t ind = triangles->size();
    for (const auto& sobj : obj_with_pos.obj.GetSubobjects()) {
        FetchTriangles(sobj, triangles);
    }
    for (const auto& triangle : obj_with_pos.obj.GetTriangles()) {
        triangles->emplace_back(triangle);
    }
    for (; ind < triangles->size(); ++ind) {
        (*triangles)[ind].vertices = obj_with_pos.pos * (*triangles)[ind].vertices;
    }
}

}  // namespace

Frame Renderer::RenderFrame(const std::vector<SubObject>& objects, const AffineTransform& camera_pos,
                            const Camera& camera, Frame&& frame) {
    triangle_buffer_.clear();
    // Да, эта функция либо через цикл, либо внутри с костылём. Единственный вариант -- изменить class World, но там
    // просто логически бред получится: в таком случае мир -- это либо бертка над единственным объектом класса Object,
    // либо контейнер, хранящий много объектов класса Object, но не имеющий своей системы координат, т.е. эти объекты
    // будут прибиты гвоздями к своему месту в мире. В данный момент мир -- это контейнер SubObject'ов, т.е. объектов с
    // их положением в пространстве, наиболее разумный вариант.
    for (const auto& sobj : objects) {
        FetchTriangles(sobj, &triangle_buffer_);
    }
    AffineTransform transformation_to_camera_space = camera_pos.inverse();

    for (Triangle& triangle : triangle_buffer_) {
        triangle.vertices = transformation_to_camera_space * triangle.vertices;
    }

    // Clipping
    ClipAgainstZAxis(camera.GetNearDist(), &triangle_buffer_);

    for (Triangle& triangle : triangle_buffer_) {
        ApplyFrustumTransformationOnTriangle(camera, &triangle);
    }

    return rasterizer_.MakeFrame(triangle_buffer_, std::move(frame));
}

}  // namespace Renderer3D::Kernel
