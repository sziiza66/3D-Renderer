#include "renderer.h"

#include <cassert>

namespace Renderer3D::Kernel {

namespace {

void ApplyFrustumTransformationOnTriangle(const Camera& cam, Triangle* triangle) {
    assert(triangle);
    triangle->vertices = cam.FrustumMatrix() * triangle->vertices;
    for (uint8_t j = 0; j < 3; ++j) {
        for (uint8_t i = 0; i < 3; ++i) {
            triangle->vertices(j, i) /= triangle->vertices(3, i);
        }
    }
    // Вернул ручную работу вместо не inplace'ного
    // triangle->vertices = triangle->vertices.colwise().hnormalized().colwise().homogeneous();
    // т.к. Eigen не предоставляет способа применять кастомные функции Col-wise.
}

void ClipTriangleCase1(double near_dist, VertexRef outside_vertex1, VertexRef outside_vertex2,
                       VertexRef inside_vertex) {
    // Этот ассёрт не должен в теории никогда срабатывать, но пусть будет.
    assert(outside_vertex1(2) - inside_vertex(2) != 0);

    outside_vertex1 = inside_vertex + (outside_vertex1 - inside_vertex) * (near_dist - inside_vertex(2)) /
                                          (outside_vertex1(2) - inside_vertex(2));
    // Этот ассёрт не должен в теории никогда срабатывать, но пусть будет.
    assert(outside_vertex2(2) - inside_vertex(2) != 0);

    outside_vertex2 = inside_vertex + (outside_vertex2 - inside_vertex) * (near_dist - inside_vertex(2)) /
                                          (outside_vertex2(2) - inside_vertex(2));
    outside_vertex1(2) = near_dist;
    outside_vertex2(2) = near_dist;
}

void ClipTriangleCase2(double near_dist, int8_t outside_vertex_ind, int8_t inside_vertex1_ind,
                       int8_t inside_vertex2_ind, Triangle* triangle, std::vector<Triangle>* triangles) {
    assert(triangles);
    assert(triangle);
    assert(outside_vertex_ind >= 0 && outside_vertex_ind < 3);
    assert(inside_vertex1_ind >= 0 && inside_vertex1_ind < 3);
    assert(inside_vertex2_ind >= 0 && inside_vertex2_ind < 3);
    // Этот ассёрт не должен в теории никогда срабатывать, но пусть будет.

    // Обработка первого треугольника
    VertexRef outside_vertex(triangle->vertices.col(outside_vertex_ind));
    VertexRef inside_vertex1(triangle->vertices.col(inside_vertex1_ind));

    assert(outside_vertex(2) - inside_vertex1(2) != 0);

    Vector4 outside_vertex_cpy = outside_vertex;

    outside_vertex = inside_vertex1 + (outside_vertex_cpy - inside_vertex1) * (near_dist - inside_vertex1(2)) /
                                          (outside_vertex_cpy(2) - inside_vertex1(2));

    outside_vertex(2) = near_dist;

    // Обработка второго треугольника
    triangles->emplace_back(*triangle);

    VertexRef outside_vertex_t2(triangles->back().vertices.col(outside_vertex_ind));
    VertexRef inside_vertex1_t2(triangles->back().vertices.col(inside_vertex1_ind));
    VertexRef inside_vertex2_t2(triangles->back().vertices.col(inside_vertex2_ind));

    // Этот ассёрт не должен в теории никогда срабатывать, но пусть будет.
    assert(outside_vertex_cpy(2) - inside_vertex2_t2(2) != 0);

    inside_vertex1_t2 = inside_vertex2_t2 + (outside_vertex_cpy - inside_vertex2_t2) *
                                                (near_dist - inside_vertex2_t2(2)) /
                                                (outside_vertex_cpy(2) - inside_vertex2_t2(2));

    assert(outside_vertex_t2(2) == near_dist);
}

std::vector<Triangle> ClipAgainstZAxis(double near_dist, std::vector<Triangle>&& triangles) {
    // Функция на разбор случаев, поэтому не особо приятная на вид, но лучше чем было.
    std::vector<Triangle> ret(std::move(triangles));
    size_t i = ret.size() - 1;
    for (; ~i; --i) {
        // Определяем, какие вершины треугольника находятся ближе, чем near plane, а какие останутся в пирамиде зрения
        // (в моем случае неограниченной, т.к. нет far plane)
        int8_t outside_vertex_ind1 = -1;
        int8_t outside_vertex_ind2 = -1;
        int8_t inside_vertex_ind1 = -1;
        int8_t inside_vertex_ind2 = -1;
        assert(i < ret.size());
        for (uint8_t j = 0; j < 3; ++j) {
            if (ret[i].vertices(2, j) < near_dist) {
                outside_vertex_ind1 = j;
                std::swap(outside_vertex_ind1, outside_vertex_ind2);
            } else {
                inside_vertex_ind1 = j;
                std::swap(inside_vertex_ind1, inside_vertex_ind2);
            }
        }
        // Весь треугольник выкидывается.
        if (inside_vertex_ind2 == -1) {
            std::swap(ret[i], ret.back());
            ret.pop_back();
            continue;
        }
        // Клиппинг не нужен.
        if (outside_vertex_ind2 == -1) {
            continue;
        }

        // Случай 1
        if (outside_vertex_ind1 != -1 && outside_vertex_ind2 != -1) {
            assert(inside_vertex_ind2 != -1);
            ClipTriangleCase1(near_dist, ret[i].vertices.col(outside_vertex_ind1),
                              ret[i].vertices.col(outside_vertex_ind2), ret[i].vertices.col(inside_vertex_ind2));
            continue;
        }
        // Случай 2
        // Функция ClipTriangleCase2 получилась не очень красивой, т.к. в этом случае появляются два новых треугольника
        // и я сэкономил пару делений и умножений. У меня в принципе много неконсистентности в этом плане, некоторые
        //
        // функций мувают в себя данные, другие принимают указатель, надеюсь это не так плохо. В целом, переписать всё
        // на передачу указателей очень просто, а вот сделать везде мувы меня как-то жаба душит, речь в основном про
        // реккурсивные функции FetchTriangles, как будто плодить там новые объекты потенциальн не очень.
        ClipTriangleCase2(near_dist, outside_vertex_ind2, inside_vertex_ind1, inside_vertex_ind2, &ret[i], &ret);
    }
    return ret;
}

void FetchAndTransformData(const std::vector<SubObject>& objects, std::vector<Triangle>* triangles,
                           std::vector<PLSInSpace>* point_lights) {
    assert(triangles);
    assert(point_lights);
    // Тут и не нужен был костыль в качестве ind = 0 дефолтного аргумента функции, я просто затупил.
    size_t ind = triangles->size();
    for (const SubObject& sobj : objects) {
        for (const Triangle& triangle : sobj.obj.Triangles()) {
            triangles->emplace_back(triangle);
        }
        for (const PointLightSource& pls : sobj.obj.PointLightSources()) {
            point_lights->emplace_back(pls);
        }
        FetchAndTransformData(sobj.obj.Subobjects(), triangles, point_lights);
        for (; ind < triangles->size(); ++ind) {
            (*triangles)[ind].vertices = sobj.pos * (*triangles)[ind].vertices;
        }
        for (; ind < point_lights->size(); ++ind) {
            (*point_lights)[ind].position += sobj.pos.translation();
        }
    }
}

}  // namespace

Frame Renderer::RenderFrame(const std::vector<SubObject>& objects, const AffineTransform& camera_pos,
                            const Camera& camera, const Color& ambient_light, Frame&& frame) {
    triangle_buffer_.clear();
    point_light_buffer_.clear();
    normal_buffer_.clear();
    preserved_buffer_.clear();
    // Да, эта функция либо через цикл, либо внутри с костылём. Единственный вариант -- изменить class World, но там
    // просто логически бред получится: в таком случае мир -- это либо бертка над единственным объектом класса Object,
    // либо контейнер, хранящий много объектов класса Object, но не имеющий своей системы координат, т.е. эти объекты
    // будут прибиты гвоздями к своему месту в мире. В данный момент мир -- это контейнер SubObject'ов, т.е. объектов с
    // их положением в пространстве, наиболее разумный вариант.
    FetchAndTransformData(objects, &triangle_buffer_, &point_light_buffer_);
    // Теперь тут обращается матрица аффинного преобразвания, а не просто Matrix4, скорее всего Eigen это оптимизирует.
    AffineTransform transformation_to_camera_space = camera_pos.inverse();

    for (Triangle& triangle : triangle_buffer_) {
        triangle.vertices = transformation_to_camera_space * triangle.vertices;
    }
    for (PLSInSpace& pls : point_light_buffer_) {
        pls.position += transformation_to_camera_space.translation();
    }

    // Clipping
    triangle_buffer_ = ClipAgainstZAxis(camera.NearDistance(), std::move(triangle_buffer_));

    // Calculating normals, temporary solution.
    for (const Triangle& triangle : triangle_buffer_) {
        Vector3 v1 = triangle.vertices.col(1).head(3) - triangle.vertices.col(0).head(3);
        Vector3 v2 = triangle.vertices.col(2).head(3) - triangle.vertices.col(0).head(3);
        normal_buffer_.emplace_back(v1.cross(v2).normalized());
    }

    for (const Triangle& triangle : triangle_buffer_) {
        preserved_buffer_.emplace_back(triangle.vertices);
    }

    for (Triangle& triangle : triangle_buffer_) {
        ApplyFrustumTransformationOnTriangle(camera, &triangle);
    }

    return rasterizer_.MakeFrame(triangle_buffer_, preserved_buffer_, point_light_buffer_, normal_buffer_, ambient_light, std::move(frame));
}

}  // namespace Renderer3D::Kernel
