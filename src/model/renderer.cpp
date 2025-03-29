#include "renderer.h"

#include <cassert>

namespace Renderer3D::Kernel {

namespace {

void ClipTriangleCase1(double near_dist, int8_t outside_vertex_ind1, int8_t outside_vertex_ind2,
                       int8_t inside_vertex_ind, Triangle* triangle) {
    assert(triangle);
    assert(outside_vertex_ind1 >= 0 && outside_vertex_ind1 < 3);
    assert(outside_vertex_ind2 >= 0 && outside_vertex_ind2 < 3);
    assert(inside_vertex_ind >= 0 && inside_vertex_ind < 3);

    VertexRef outside_vertex1 = triangle->vertices.col(outside_vertex_ind1);
    VertexRef outside_vertex2 = triangle->vertices.col(outside_vertex_ind2);
    VertexRef inside_vertex = triangle->vertices.col(inside_vertex_ind);
    NormalRef outside_normal1 = triangle->vertex_normals.col(outside_vertex_ind1);
    NormalRef outside_normal2 = triangle->vertex_normals.col(outside_vertex_ind2);
    NormalRef inside_normal = triangle->vertex_normals.col(inside_vertex_ind);

    // Этот ассёрт не должен в теории никогда срабатывать, но пусть будет.
    assert(outside_vertex1(2) - inside_vertex(2) != 0);

    outside_normal1 = inside_normal + (outside_normal1 - inside_normal) * (near_dist - inside_vertex(2)) /
                                          (outside_vertex1(2) - inside_vertex(2));

    outside_vertex1 = inside_vertex + (outside_vertex1 - inside_vertex) * (near_dist - inside_vertex(2)) /
                                          (outside_vertex1(2) - inside_vertex(2));
    // Этот ассёрт не должен в теории никогда срабатывать, но пусть будет.
    assert(outside_vertex2(2) - inside_vertex(2) != 0);

    outside_normal2 = inside_normal + (outside_normal2 - inside_normal) * (near_dist - inside_vertex(2)) /
                                          (outside_vertex2(2) - inside_vertex(2));

    outside_vertex2 = inside_vertex + (outside_vertex2 - inside_vertex) * (near_dist - inside_vertex(2)) /
                                          (outside_vertex2(2) - inside_vertex(2));
    outside_vertex1(2) = near_dist;
    outside_vertex2(2) = near_dist;
}

Triangle ClipTriangleCase2(double near_dist, int8_t outside_vertex_ind, int8_t inside_vertex_ind1,
                           int8_t inside_vertex_ind2, Triangle* triangle) {
    assert(triangle);
    assert(outside_vertex_ind >= 0 && outside_vertex_ind < 3);
    assert(inside_vertex_ind1 >= 0 && inside_vertex_ind1 < 3);
    assert(inside_vertex_ind2 >= 0 && inside_vertex_ind2 < 3);
    // Этот ассёрт не должен в теории никогда срабатывать, но пусть будет.

    // Обработка первого треугольника
    VertexRef outside_vertex(triangle->vertices.col(outside_vertex_ind));
    VertexRef inside_vertex1(triangle->vertices.col(inside_vertex_ind1));
    NormalRef outside_normal(triangle->vertex_normals.col(outside_vertex_ind));
    NormalRef inside_normal1(triangle->vertex_normals.col(inside_vertex_ind1));

    assert(outside_vertex(2) - inside_vertex1(2) != 0);

    Vector4 outside_vertex_cpy = outside_vertex;
    Vector3 outside_normal_cpy = outside_normal;

    outside_normal = inside_normal1 + (outside_normal_cpy - inside_normal1) * (near_dist - inside_vertex1(2)) /
                                          (outside_vertex_cpy(2) - inside_vertex1(2));

    outside_vertex = inside_vertex1 + (outside_vertex_cpy - inside_vertex1) * (near_dist - inside_vertex1(2)) /
                                          (outside_vertex_cpy(2) - inside_vertex1(2));

    outside_vertex(2) = near_dist;

    // Обработка второго треугольника
    Triangle ret = *triangle;

    VertexRef outside_vertex_t2(ret.vertices.col(outside_vertex_ind));
    VertexRef inside_vertex1_t2(ret.vertices.col(inside_vertex_ind1));
    VertexRef inside_vertex2_t2(ret.vertices.col(inside_vertex_ind2));
    NormalRef outside_normal_t2(ret.vertex_normals.col(outside_vertex_ind));
    NormalRef inside_normal1_t2(ret.vertex_normals.col(inside_vertex_ind1));
    NormalRef inside_normal2_t2(ret.vertex_normals.col(inside_vertex_ind2));

    // Этот ассёрт не должен в теории никогда срабатывать, но пусть будет.
    assert(outside_vertex_cpy(2) - inside_vertex2_t2(2) != 0);

    inside_normal1_t2 = inside_normal2_t2 + (outside_normal_cpy - inside_normal2_t2) *
                                                (near_dist - inside_vertex2_t2(2)) /
                                                (outside_vertex_cpy(2) - inside_vertex2_t2(2));

    inside_vertex1_t2 = inside_vertex2_t2 + (outside_vertex_cpy - inside_vertex2_t2) *
                                                (near_dist - inside_vertex2_t2(2)) /
                                                (outside_vertex_cpy(2) - inside_vertex2_t2(2));
    Vector4 temp_vertex = inside_vertex1_t2;
    inside_vertex1_t2 = inside_vertex2_t2;
    inside_vertex2_t2 = temp_vertex;

    Vector3 temp_normal = inside_normal1_t2;
    inside_normal1_t2 = inside_normal2_t2;
    inside_normal2_t2 = temp_normal;

    assert(outside_vertex_t2(2) == near_dist);

    return ret;
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
            ClipTriangleCase1(near_dist, outside_vertex_ind1, outside_vertex_ind2, inside_vertex_ind2, &ret[i]);
            continue;
        }
        // Случай 2
        // Функция ClipTriangleCase2 получилась не очень красивой, т.к. в этом случае появляются два новых треугольника
        // и я сэкономил пару делений и умножений. У меня в принципе много неконсистентности в этом плане, некоторые
        //
        // функций мувают в себя данные, другие принимают указатель, надеюсь это не так плохо. В целом, переписать всё
        // на передачу указателей очень просто, а вот сделать везде мувы меня как-то жаба душит, речь в основном про
        // реккурсивные функции FetchTriangles, как будто плодить там новые объекты потенциальн не очень.
        ret.emplace_back(
            ClipTriangleCase2(near_dist, outside_vertex_ind2, inside_vertex_ind1, inside_vertex_ind2, &ret[i]));
    }
    return ret;
}

void FetchAndTransformData(const std::vector<SubObject>& objects, std::vector<Triangle>* triangles,
                           std::vector<PLSInSpace>* point_lights) {
    assert(triangles);
    assert(point_lights);
    // Тут и не нужен был костыль в качестве ind = 0 дефолтного аргумента функции, я просто затупил.
    size_t ind_tri = triangles->size();
    size_t ind_pls = point_lights->size();
    for (const SubObject& sobj : objects) {
        for (const Triangle& triangle : sobj.obj.Triangles()) {
            triangles->emplace_back(triangle);
        }
        for (const PointLightSource& pls : sobj.obj.PointLightSources()) {
            point_lights->emplace_back(pls);
        }
        FetchAndTransformData(sobj.obj.Subobjects(), triangles, point_lights);
        for (; ind_tri < triangles->size(); ++ind_tri) {
            (*triangles)[ind_tri].vertices = sobj.pos * (*triangles)[ind_tri].vertices;
            (*triangles)[ind_tri].vertex_normals = sobj.pos.rotation() * (*triangles)[ind_tri].vertex_normals;
        }
        for (; ind_pls < point_lights->size(); ++ind_pls) {
            (*point_lights)[ind_pls].position += sobj.pos.translation();
        }
    }
}

}  // namespace

Frame Renderer::RenderFrame(const std::vector<SubObject>& objects, const AffineTransform& camera_pos,
                            const Camera& camera, const Color& ambient_light, Frame&& frame) {
    triangle_buffer_.clear();
    point_light_buffer_.clear();
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
        triangle.vertex_normals = transformation_to_camera_space.rotation() * triangle.vertex_normals;
    }
    for (PLSInSpace& pls : point_light_buffer_) {
        pls.position += transformation_to_camera_space.translation();
    }

    // Clipping // надо переделать нормали
    triangle_buffer_ = ClipAgainstZAxis(camera.NearDistance(), std::move(triangle_buffer_));

    return rasterizer_.MakeFrame(triangle_buffer_, point_light_buffer_, ambient_light, camera, std::move(frame));
}

}  // namespace Renderer3D::Kernel
