#include "rasterizer.h"

#include <cassert>
#include <iostream>

#include "zbuffer.h"

namespace Renderer3D::Kernel {
namespace {

constexpr double kSideOfTheCube = 2.0;

auto GetVerticalOrderOfVerticesAndAttributes(const TriMatrix& projected_vertices, const Triangle& triangle) {
    std::array<uint8_t, 3> ret = {0, 1, 2};

    // Eigen::Block не swap'ается :(
    if (projected_vertices(0, ret[2]) < projected_vertices(0, ret[1])) {
        std::swap(ret[2], ret[1]);
    }
    if (projected_vertices(0, ret[2]) < projected_vertices(0, ret[0])) {
        std::swap(ret[2], ret[0]);
    }
    if (projected_vertices(0, ret[1]) < projected_vertices(0, ret[0])) {
        std::swap(ret[1], ret[0]);
    }
    // Ужас
    return std::tuple<ConstVertexRef, ConstVertexRef, ConstVertexRef, Vector3, Vector3, Vector3, Vector3, Vector3,
                      Vector3>{projected_vertices.col(ret[0]),        projected_vertices.col(ret[1]),
                               projected_vertices.col(ret[2]),        triangle.vertices.col(ret[0]).head(3),
                               triangle.vertices.col(ret[1]).head(3), triangle.vertices.col(ret[2]).head(3),
                               triangle.vertex_normals.col(ret[0]),   triangle.vertex_normals.col(ret[1]),
                               triangle.vertex_normals.col(ret[2])};
}

Color CalculateLightIntensityColor(const PointLightSource& source, double d2) {
    return source.color / (std::abs(source.k_const + source.k_linear * std::sqrt(d2) + source.k_quadr * d2));
}

DiscreteColor CalculateColorOfPizxel(const Color& diffuse_color, const Color& ambient,
                                     const std::vector<PLSInSpace>& pls, const Vector3& point, const Vector3& normal) {
    Color ret = diffuse_color;
    Color modulator = ambient;
    for (const PLSInSpace& pl : pls) {
        Vector3 distance_vector = point - pl.position;
        double dotprod = std::abs(normal.dot(distance_vector.normalized()));
        modulator += dotprod * CalculateLightIntensityColor(pl.source_data, distance_vector.squaredNorm());
    }
    return MakeDiscrete(ret * modulator);
}

Vector3 InterpolatePointBack(const Vector3& interpolated_point) {
    double z = 1.0 / interpolated_point(2);
    return {z * interpolated_point(0), z * interpolated_point(1), z};
}

Vector3 InterpolateNormalBack(const Vector3& interpolated_normal, double z) {
    return z * interpolated_normal;
}

void FillSegment(const Color& diffuse_color, const Color& ambient, const std::vector<PLSInSpace>& pls,
                 const Vector3& interpolated_point1, const Vector3& interpolated_point2,
                 const Vector3& interpolated_normal1, const Vector3& interpolated_normal2, double real_y1,
                 double real_y2, size_t x, double real_z, double z_diff_y, double real_z_diff_y, Frame* frame,
                 ZBuffer* z_buffer_) {
    Vector3 interpolated_point = interpolated_point1;
    Vector3 interpolated_point_inc = (interpolated_point2 - interpolated_point1) / (real_y2 - real_y1);
    Vector3 interpolated_normal = interpolated_normal1;
    Vector3 interpolated_normal_inc = (interpolated_normal2 - interpolated_normal1) / (real_y2 - real_y1);
    if (real_y1 < -1) {
        interpolated_point += interpolated_point_inc * (-real_y1 - 1);
        interpolated_normal += interpolated_normal_inc * (-real_y1 - 1);
        real_z += (-real_y1 - 1) * real_z_diff_y;
        real_y1 = -1;
    }

    size_t y1 = ((real_y1 + 1) / kSideOfTheCube) * frame->Width();
    size_t y2 = ((real_y2 + 1) / kSideOfTheCube) * frame->Width() + 1;
    assert(y1 < y2);

    size_t edge = (y2 >= frame->Width() ? frame->Width() : y2);

    for (size_t y = y1; y != (y2 >= edge ? edge : y2); ++y, real_z += z_diff_y) {
        if (real_z < (*z_buffer_)(x, y)) {
            (*z_buffer_)(x, y) = real_z;
            Vector3 point = InterpolatePointBack(interpolated_point);
            (*frame)(x, y) = CalculateColorOfPizxel(diffuse_color, ambient, pls, point,
                                                    InterpolateNormalBack(interpolated_normal, point(2)));
        }
        interpolated_point += interpolated_point_inc * kSideOfTheCube / frame->Width();
        interpolated_normal += interpolated_normal_inc * kSideOfTheCube / frame->Width();
    }
}

void FillLowerTriangle(const Color& diffuse_color, const Color& ambient, const std::vector<PLSInSpace>& pls,
                       ConstVertexRef lowest_proj, ConstVertexRef middle_proj, ConstVertexRef highest_proj,
                       const Vector3& lowest, const Vector3& middle, const Vector3& highest, const Vector3& lowest_norm,
                       const Vector3& middle_norm, const Vector3& highest_norm, double real_z_diff_y, double z_diff_y,
                       double z_diff_x, Frame* frame, ZBuffer* z_buffer_, double* real_x, double* real_z, size_t* x,
                       double* prev_y) {
    double mid_x = (middle_proj(0) <= 1 ? middle_proj(0) : 1);
    double dx = kSideOfTheCube / frame->Height();
    for (; *real_x < mid_x; ++(*x), *real_x += dx, *real_z += z_diff_x) {
        // real_y1, real_y2 -- y координаты отрезка в видимом пространстве, который будет нарисован на экране.
        double real_y1 = highest_proj(0) == lowest_proj(0)
                             ? highest_proj(1)
                             : lowest_proj(1) + (highest_proj(1) - lowest_proj(1)) * (*real_x - lowest_proj(0)) /
                                                    (highest_proj(0) - lowest_proj(0));
        double real_y2 = lowest_proj(0) == middle_proj(0)
                             ? middle_proj(1)
                             : lowest_proj(1) + (middle_proj(1) - lowest_proj(1)) * (*real_x - lowest_proj(0)) /
                                                    (middle_proj(0) - lowest_proj(0));

        if ((real_y2 < -1 && real_y1 < -1) || (real_y1 > 1 && real_y2 > 1)) {
            continue;
        }

        // Я знаю, как избавиться от всех операций деления в этой функции, но пока оставлю так.
        // Интерполяция аттрибутов
        // --------------------
        // x, y, z координаты
        Vector3 interpolated_point1 =
            highest_proj(0) == lowest_proj(0)
                ? highest
                : lowest + (highest - lowest) * (*real_x - lowest_proj(0)) / (highest_proj(0) - lowest_proj(0));
        Vector3 interpolated_point2 =
            middle_proj(0) == lowest_proj(0)
                ? middle
                : lowest + (middle - lowest) * (*real_x - lowest_proj(0)) / (middle_proj(0) - lowest_proj(0));
        // нормали
        Vector3 interpolated_normal1 = highest_proj(0) == lowest_proj(0)
                                           ? highest_norm
                                           : lowest_norm + (highest_norm - lowest_norm) * (*real_x - lowest_proj(0)) /
                                                               (highest_proj(0) - lowest_proj(0));
        Vector3 interpolated_normal2 = middle_proj(0) == lowest_proj(0)
                                           ? middle_norm
                                           : lowest_norm + (middle_norm - lowest_norm) * (*real_x - lowest_proj(0)) /
                                                               (middle_proj(0) - lowest_proj(0));
        // --------------------

        if (real_y1 > real_y2) {
            std::swap(real_y1, real_y2);
            // Данные тоже нужно свапнуть
            std::swap(interpolated_point1, interpolated_point2);
            std::swap(interpolated_normal1, interpolated_normal2);
        }

        *real_z += real_z_diff_y * (real_y1 - *prev_y);
        *prev_y = real_y1;

        // Рисуем этот отрезок
        FillSegment(diffuse_color, ambient, pls, interpolated_point1, interpolated_point2, interpolated_normal1,
                    interpolated_normal2, real_y1, real_y2, *x, *real_z, z_diff_y, real_z_diff_y, frame, z_buffer_);
    }
}

void FillUpperTriangle(const Color& diffuse_color, const Color& ambient, const std::vector<PLSInSpace>& pls,
                       ConstVertexRef lowest_proj, ConstVertexRef middle_proj, ConstVertexRef highest_proj,
                       const Vector3& lowest, const Vector3& middle, const Vector3& highest, const Vector3& lowest_norm,
                       const Vector3& middle_norm, const Vector3& highest_norm, double real_z_diff_y, double z_diff_y,
                       double z_diff_x, Frame* frame, ZBuffer* z_buffer_, double* real_x, double* real_z, size_t* x,
                       double* prev_y) {
    double top_x = (highest_proj(0) <= 1 ? highest_proj(0) : 1);
    double dx = kSideOfTheCube / frame->Height();
    for (; *real_x < top_x; ++(*x), *real_x += dx, *real_z += z_diff_x) {
        // real_y1, real_y2 -- y координаты отрезка в видимом пространстве, который будет нарисован на экране.
        double real_y1 = highest_proj(0) == lowest_proj(0)
                             ? highest_proj(1)
                             : lowest_proj(1) + (highest_proj(1) - lowest_proj(1)) * (*real_x - lowest_proj(0)) /
                                                    (highest_proj(0) - lowest_proj(0));
        double real_y2 = highest_proj(0) == middle_proj(0)
                             ? highest_proj(1)
                             : middle_proj(1) + (highest_proj(1) - middle_proj(1)) * (*real_x - middle_proj(0)) /
                                                    (highest_proj(0) - middle_proj(0));

        if ((real_y2 < -1 && real_y1 < -1) || (real_y1 > 1 && real_y2 > 1)) {
            continue;
        }

        // Я знаю, как избавиться от всех операций деления в этой функции, но пока оставлю так.
        // Интерполяция аттрибутов
        // --------------------
        // x, y, z координаты
        Vector3 interpolated_point1 =
            highest_proj(0) == lowest_proj(0)
                ? highest
                : lowest + (highest - lowest) * (*real_x - lowest_proj(0)) / (highest_proj(0) - lowest_proj(0));
        Vector3 interpolated_point2 =
            highest_proj(0) == middle_proj(0)
                ? highest
                : middle + (highest - middle) * (*real_x - middle_proj(0)) / (highest_proj(0) - middle_proj(0));
        // нормали
        Vector3 interpolated_normal1 = highest_proj(0) == lowest_proj(0)
                                           ? highest_norm
                                           : lowest_norm + (highest_norm - lowest_norm) * (*real_x - lowest_proj(0)) /
                                                               (highest_proj(0) - lowest_proj(0));
        Vector3 interpolated_normal2 = highest_proj(0) == middle_proj(0)
                                           ? highest_norm
                                           : middle_norm + (highest_norm - middle_norm) * (*real_x - middle_proj(0)) /
                                                               (highest_proj(0) - middle_proj(0));
        // --------------------

        if (real_y1 > real_y2) {
            std::swap(real_y1, real_y2);
            // Данные тоже нужно свапнуть
            std::swap(interpolated_point1, interpolated_point2);
            std::swap(interpolated_normal1, interpolated_normal2);
        }

        *real_z += real_z_diff_y * (real_y1 - *prev_y);
        *prev_y = real_y1;

        // Рисуем этот отрезок
        FillSegment(diffuse_color, ambient, pls, interpolated_point1, interpolated_point2, interpolated_normal1,
                    interpolated_normal2, real_y1, real_y2, *x, *real_z, z_diff_y, real_z_diff_y, frame, z_buffer_);
    }
}

void DrawTriangle(const Triangle& triangle, TriMatrix projected_vertices, const std::vector<PLSInSpace>& pls,
                  const Color& ambient, Frame* frame, ZBuffer* z_buffer_) {
    // Eigen::Block'и вершин треугольника, отсортированные по Ox.
    auto [lowest_proj, middle_proj, highest_proj, lowest, middle, highest, lowest_norm, middle_norm, highest_norm] =
        GetVerticalOrderOfVerticesAndAttributes(projected_vertices, triangle);

    /*
        Вроде это единственное место, где я конвертирую Vector4 -> Vector3 или наоборот, если реально стоит выделить
        конвертацию в функцию, то без проблем. В любом случае я сделаю read/write сцены в файл, а в файле я буду хранить
        матрицы компактно, так что наверное что-то такое понадобится.
    */
    // Получаю векторы, напраленные из нижней (по Ox) точки треугольника в среднюю и верхнюю соотв.
    Vector3 v1 = middle_proj.head(3) - lowest_proj.head(3);
    Vector3 v2 = highest_proj.head(3) - lowest_proj.head(3);
    // Получаю вектр нормали к плоскости треугольника.
    v1 = v1.cross(v2);

    // Смысл происходящего далее такой: я поддерживаю две точки, соответствующие друг другу, точку на треугольнике в
    // нашем видимом пространстве и точку треугольника на экране. Я двигаю точку на экране по оси Ox и Oy, при этом в
    // нужной пропорции изменяя координату точки в видимом пространстве с помощью посчитанных здесь коэффициентов.
    // real_z_diff_y -- Коэффициент изменения координаты z при изменении координаты y (т.е. такое, чтобы снова попасть в
    // плоскость треугольника) в видимом пространстве.
    // z_diff_y -- Коэффициент изменения координаты z при изменении координаты y на такое значение, чтобы кордината y на
    // экране сдвинулась на один пиксель.
    // z_diff_x -- аналогично для x.
    // real_x, real_y, real_z -- координаты поддреживаемой точки в видимом пространстве.
    double real_z_diff_y = (v1(2) == 0 ? 0 : -v1(1) / v1(2));
    double z_diff_y = real_z_diff_y * kSideOfTheCube / frame->Width();
    double z_diff_x = (v1(2) == 0 ? 0 : -v1(0) / v1(2) * kSideOfTheCube / frame->Height());

    // Я передаю эти четыре переменные по указателю как изменяемые входные данные. Я мб позже сменю это на return tuple
    // или struct, мне нужно подумать.
    double real_x = (lowest_proj(0) >= -1 ? lowest_proj(0) : -1);
    double prev_y = lowest_proj(1);
    double real_z = lowest_proj(2) - (v1(2) == 0 ? 0 : v1(0) / v1(2)) * (real_x - lowest_proj(0));
    size_t x = frame->Height() * ((real_x + 1) / kSideOfTheCube);
    x = (x >= frame->Height() ? frame->Height() - 1 : x);

    // Интерполяция аттрибутов
    // --------------------
    assert(lowest(2) != 0);
    assert(middle(2) != 0);
    assert(highest(2) != 0);
    // z координата
    lowest(2) = 1.0 / lowest(2);
    middle(2) = 1.0 / middle(2);
    highest(2) = 1.0 / highest(2);
    // x, y координаты
    lowest(0) *= lowest(2);
    lowest(1) *= lowest(2);
    middle(0) *= middle(2);
    middle(1) *= middle(2);
    highest(0) *= highest(2);
    highest(1) *= highest(2);
    // нормали
    lowest_norm *= lowest(2);
    middle_norm *= middle(2);
    highest_norm *= highest(2);
    // --------------------

    // Треугольник разбивается на два других с одной из сторон параллельной Oy. Далее отриссовываем эти треугольники на
    // экране соотв. функциями.
    // Я не уверен, нужно ли всё-таки менять название функции, теперь видно, что меняется?
    FillLowerTriangle(triangle.diffuse_reflection_color, ambient, pls, lowest_proj, middle_proj, highest_proj, lowest,
                      middle, highest, lowest_norm, middle_norm, highest_norm, real_z_diff_y, z_diff_y, z_diff_x, frame,
                      z_buffer_, &real_x, &real_z, &x, &prev_y);

    FillUpperTriangle(triangle.diffuse_reflection_color, ambient, pls, lowest_proj, middle_proj, highest_proj, lowest,
                      middle, highest, lowest_norm, middle_norm, highest_norm, real_z_diff_y, z_diff_y, z_diff_x, frame,
                      z_buffer_, &real_x, &real_z, &x, &prev_y);
}

TriMatrix ApplyFrustumTransformationOnTriangle(const Triangle& triangle, const Camera& cam) {
    TriMatrix ret = cam.FrustumMatrix() * triangle.vertices;
    for (uint8_t j = 0; j < 3; ++j) {
        for (uint8_t i = 0; i < 3; ++i) {
            ret(j, i) /= ret(3, i);
        }
    }
    // Вернул ручную работу вместо не inplace'ного
    // triangle->vertices = triangle->vertices.colwise().hnormalized().colwise().homogeneous();
    // т.к. Eigen не предоставляет способа применять кастомные функции Col-wise.
    return ret;
}

}  // namespace

size_t fcnt = 0;

Frame BufferRasterizer::MakeFrame(const std::vector<Triangle>& triangles, const std::vector<PLSInSpace>& pls,
                                  const Color& ambient, const Camera& camera, Frame&& frame) {
    Frame ret(std::move(frame));
    z_buffer_.FitTo(ret);
    ret.FillWithBlackColor();
    for (const Triangle& triangle : triangles) {
        // В целях оптимизации проективное преобразование пришлось перенести сюда.
        TriMatrix projected_vertices = ApplyFrustumTransformationOnTriangle(triangle, camera);
        DrawTriangle(triangle, projected_vertices, pls, ambient, &ret, &z_buffer_);
    }
    std::cout << "Frame " << fcnt++ << " rendered!\n";
    return ret;
}

}  // namespace Renderer3D::Kernel
