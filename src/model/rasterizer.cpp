#include "rasterizer.h"

#include <cassert>
#include <cmath>

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
    return std::tuple<Vector3, Vector3, Vector3, Vector3, Vector3, Vector3, Vector3, Vector3, Vector3>{
        projected_vertices.col(ret[0]).head(3), projected_vertices.col(ret[1]).head(3),
        projected_vertices.col(ret[2]).head(3), triangle.vertices.col(ret[0]).head(3),
        triangle.vertices.col(ret[1]).head(3),  triangle.vertices.col(ret[2]).head(3),
        triangle.vertex_normals.col(ret[0]),    triangle.vertex_normals.col(ret[1]),
        triangle.vertex_normals.col(ret[2])};
}

double ApplyBinPow(double x, uint32_t pow) {
    double ret = 1;
    do {
        ret *= (pow & 1 ? x : 1);
        pow >>= 1;
        ret *= (pow ? ret : 1);
    } while (pow);
    return ret;
}

Color CalculatePointLightIntensityColor(const Color& color, double k_const, double k_linear, double k_quadr,
                                        double distance_squared) {
    assert(k_const + k_linear * std::sqrt(distance_squared) + k_quadr * distance_squared != 0);
    return color / (std::abs(k_const + k_linear * std::sqrt(distance_squared) + k_quadr * distance_squared));
}

void ApplyPointLighting(const PLSInSpace& pls, uint32_t specular_pow, const Vector3& point, const Vector3& normal,
                        Color* diffuse_modulator, Color* specular_modulator) {
    Vector3 distance_vector = pls.position - point;
    double diffuse_coef = normal.dot(distance_vector.normalized());
    diffuse_coef = (diffuse_coef > 0 ? diffuse_coef : 0);
    Color pl_intensity =
        CalculatePointLightIntensityColor(pls.source_data.color, pls.source_data.k_const, pls.source_data.k_linear,
                                          pls.source_data.k_quadr, distance_vector.squaredNorm());
    *diffuse_modulator += diffuse_coef * pl_intensity;

    if (diffuse_coef <= 0) {
        return;
    }
    double specular_coef = normal.dot((distance_vector - point).normalized());
    specular_coef = ApplyBinPow((specular_coef > 0 ? specular_coef : 0), specular_pow);
    *specular_modulator += diffuse_coef * specular_coef * pl_intensity;
}

void ApplySpotLighting(const SLSInSpace& sls, uint32_t specular_pow, const Vector3& point, const Vector3& normal,
                       Color* diffuse_modulator, Color* specular_modulator) {
    Vector3 distance_vector = sls.position - point;
    double diffuse_coef = normal.dot(distance_vector.normalized());
    diffuse_coef = (diffuse_coef > 0 ? diffuse_coef : 0);
    double concentration_coef = -sls.source_data.direction.dot(distance_vector.normalized());
    concentration_coef = (concentration_coef > 0 ? concentration_coef : 0);
    concentration_coef = ApplyBinPow(concentration_coef, sls.source_data.concentration);
    Color pl_intensity =
        concentration_coef * CalculatePointLightIntensityColor(sls.source_data.color, sls.source_data.k_const,
                                                               sls.source_data.k_linear, sls.source_data.k_quadr,
                                                               distance_vector.squaredNorm());
    *diffuse_modulator += diffuse_coef * pl_intensity;

    if (diffuse_coef <= 0) {
        return;
    }
    double specular_coef = normal.dot((distance_vector - point).normalized());
    specular_coef = ApplyBinPow((specular_coef > 0 ? specular_coef : 0), specular_pow);
    *specular_modulator += diffuse_coef * specular_coef * pl_intensity;
}

void ApplyDirectionalLighting(const DirectionalLightSource& dls, uint32_t specular_pow, const Vector3& normal,
                              Color* diffuse_modulator, Color* specular_modulator) {
    double diffuse_coef = normal.dot(dls.direction);
    diffuse_coef = (diffuse_coef > 0 ? diffuse_coef : 0);

    *diffuse_modulator += diffuse_coef * dls.color;

    if (diffuse_coef <= 0) {
        return;
    }
    double specular_coef = normal.dot(dls.direction);
    specular_coef = ApplyBinPow((specular_coef > 0 ? specular_coef : 0), specular_pow);
    *specular_modulator += diffuse_coef * specular_coef * dls.color;
}

DiscreteColor CalculateColorOfPixel(const Color& diffuse_color, const Color& specular_color, const Color& ambient,
                                    const std::vector<DirectionalLightSource>& directional_lights,
                                    uint32_t specular_pow, const std::vector<PLSInSpace>& point_lights,
                                    const std::vector<SLSInSpace>& spot_lights, const Vector3& point,
                                    const Vector3& normal) {
    Color diffuse_modulator = ambient;
    Color specular_modulator = {0, 0, 0};
    for (const PLSInSpace& pls : point_lights) {
        ApplyPointLighting(pls, specular_pow, point, normal, &diffuse_modulator, &specular_modulator);
    }

    for (const SLSInSpace& sls : spot_lights) {
        ApplySpotLighting(sls, specular_pow, point, normal, &diffuse_modulator, &specular_modulator);
    }

    for (const DirectionalLightSource& dls : directional_lights) {
        ApplyDirectionalLighting(dls, specular_pow, normal, &diffuse_modulator, &specular_modulator);
    }

    return MakeDiscrete(diffuse_color * diffuse_modulator + specular_color * specular_modulator);
}

Vector3 InterpolatePointBack(const Vector3& interpolated_point) {
    double z = 1.0 / interpolated_point(2);
    return {z * interpolated_point(0), z * interpolated_point(1), z};
}

void FillSegment(const Color& diffuse_color, const Color& specular_color, const Color& ambient,
                 const std::vector<DirectionalLightSource>& directional_lights, uint32_t specular_pow,
                 const std::vector<PLSInSpace>& point_lights, const std::vector<SLSInSpace>& spot_lights,
                 const Vector3& interpolated_point1, const Vector3& interpolated_point2,
                 const Vector3& interpolated_normal1, const Vector3& interpolated_normal2, double real_y1,
                 double real_y2, size_t x, double real_z, double z_diff_y, double real_z_diff_y, Frame* frame,
                 ZBuffer* z_buffer_) {
    assert(x < frame->Height());

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
    size_t edge = (y2 >= frame->Width() ? frame->Width() : y2);
    assert(y1 < edge);

    for (size_t y = y1; y != edge; ++y, real_z += z_diff_y) {
        if (real_z < (*z_buffer_)(x, y)) {
            (*z_buffer_)(x, y) = real_z;
            Vector3 point = InterpolatePointBack(interpolated_point);
            (*frame)(x, y) =
                CalculateColorOfPixel(diffuse_color, specular_color, ambient, directional_lights, specular_pow,
                                      point_lights, spot_lights, point, (interpolated_normal * point(2)).normalized());
        }
        interpolated_point += interpolated_point_inc * kSideOfTheCube / frame->Width();
        interpolated_normal += interpolated_normal_inc * kSideOfTheCube / frame->Width();
    }
}

double CalcVecProdXY(const Vector3& v1, const Vector3& v2) {
    return v1(0) * v2(1) - v1(1) * v2(0);
}

std::tuple<double, double, double, size_t> FillLowerTriangle(
    const Color& diffuse_color, const Color& specular_color, const Color& ambient,
    const std::vector<DirectionalLightSource>& directional_lights, uint32_t specular_pow,
    const std::vector<PLSInSpace>& point_lights, const std::vector<SLSInSpace>& spot_lights, const Vector3& lowest_proj,
    Vector3 middle_proj, Vector3 highest_proj, const Vector3& lowest, Vector3 middle, Vector3 highest,
    const Vector3& lowest_norm, Vector3 middle_norm, Vector3 highest_norm, double real_z_diff_y, double z_diff_y,
    double real_x, size_t x, Frame* frame, ZBuffer* z_buffer_) {
    double dx = kSideOfTheCube / frame->Height();
    double mid_x = (middle_proj(0) <= 1 ? middle_proj(0) : 1);

    if (CalcVecProdXY(middle_proj - lowest_proj, highest_proj - lowest_proj) > 0) {
        std::swap(middle_proj, highest_proj);
        std::swap(middle, highest);
        std::swap(middle_norm, highest_norm);
    }

    double prev_real_y1 = std::numeric_limits<double>::infinity();
    double prev_real_y2 = -std::numeric_limits<double>::infinity();
    for (; real_x < mid_x; ++x, real_x += dx) {
        double left_coeff =
            highest_proj(0) == lowest_proj(0) ? 1 : (real_x - lowest_proj(0)) / (highest_proj(0) - lowest_proj(0));
        double right_coeff =
            lowest_proj(0) == middle_proj(0) ? 1 : (real_x - lowest_proj(0)) / (middle_proj(0) - lowest_proj(0));

        // real_y1, real_y2 -- y координаты отрезка в видимом пространстве, который будет нарисован на экране.
        double real_y1 = lowest_proj(1) + (highest_proj(1) - lowest_proj(1)) * left_coeff;
        double real_y2 = lowest_proj(1) + (middle_proj(1) - lowest_proj(1)) * right_coeff;

        double actual_real_y1 = real_y1 < prev_real_y1 ? real_y1 : prev_real_y1;
        double actual_real_y2 = real_y2 > prev_real_y2 ? real_y2 : prev_real_y2;

        if (actual_real_y1 > 1 || actual_real_y2 < -1) {
            continue;
        }

        // Я знаю, как избавиться от всех операций деления в этой функции, но пока оставлю так.
        // Интерполяция аттрибутов
        // --------------------
        // x, y, z координаты
        Vector3 interpolated_point1 = lowest + (highest - lowest) * left_coeff;
        Vector3 interpolated_point2 = lowest + (middle - lowest) * right_coeff;
        // нормали
        // Здесь интерполирую через y, потому что такой способ убирает визуальные баги
        Vector3 interpolated_normal1 = highest_proj(1) == lowest_proj(1)
                                           ? highest_norm
                                           : lowest_norm + (highest_norm - lowest_norm) *
                                                               (actual_real_y1 - lowest_proj(1)) /
                                                               (highest_proj(1) - lowest_proj(1));
        Vector3 interpolated_normal2 = middle_proj(1) == lowest_proj(1)
                                           ? middle_norm
                                           : lowest_norm + (middle_norm - lowest_norm) *
                                                               (actual_real_y2 - lowest_proj(1)) /
                                                               (middle_proj(1) - lowest_proj(1));
        // --------------------

        double real_z = lowest_proj(2) + (highest_proj(2) - lowest_proj(2)) * left_coeff;

        // Рисуем этот отрезок
        FillSegment(diffuse_color, specular_color, ambient, directional_lights, specular_pow, point_lights, spot_lights,
                    interpolated_point1, interpolated_point2, interpolated_normal1, interpolated_normal2,
                    actual_real_y1, actual_real_y2, x, real_z, z_diff_y, real_z_diff_y, frame, z_buffer_);
        prev_real_y1 = real_y1;
        prev_real_y2 = real_y2;
    }
    return {prev_real_y1, prev_real_y2, real_x, x};
}

void FillUpperTriangle(const Color& diffuse_color, const Color& specular_color, const Color& ambient,
                       const std::vector<DirectionalLightSource>& directional_lights, uint32_t specular_pow,
                       const std::vector<PLSInSpace>& point_lights, const std::vector<SLSInSpace>& spot_lights,
                       Vector3 lowest_proj, Vector3 middle_proj, const Vector3& highest_proj, Vector3 lowest,
                       Vector3 middle, const Vector3& highest, Vector3 lowest_norm, Vector3 middle_norm,
                       const Vector3& highest_norm, double real_z_diff_y, double z_diff_y, double prev_real_y1,
                       double prev_real_y2, double real_x, size_t x, Frame* frame, ZBuffer* z_buffer_) {
    double dx = kSideOfTheCube / frame->Height();
    double top_x = (highest_proj(0) <= 1 ? highest_proj(0) : 1);

    if (CalcVecProdXY(highest_proj - middle_proj, highest_proj - lowest_proj) < 0) {
        std::swap(middle_proj, lowest_proj);
        std::swap(middle, lowest);
        std::swap(middle_norm, lowest_norm);
    }

    for (; real_x <= top_x; ++x, real_x += dx) {
        // real_y1, real_y2 -- y координаты отрезка в видимом пространстве, который будет нарисован на экране.
        double left_coeff =
            highest_proj(0) == lowest_proj(0) ? 1 : (real_x - lowest_proj(0)) / (highest_proj(0) - lowest_proj(0));
        double right_coeff =
            highest_proj(0) == middle_proj(0) ? 1 : (real_x - middle_proj(0)) / (highest_proj(0) - middle_proj(0));

        double real_y1 = lowest_proj(1) + (highest_proj(1) - lowest_proj(1)) * left_coeff;
        double real_y2 = middle_proj(1) + (highest_proj(1) - middle_proj(1)) * right_coeff;

        double actual_real_y1 = real_y1 < prev_real_y1 ? real_y1 : prev_real_y1;
        double actual_real_y2 = real_y2 > prev_real_y2 ? real_y2 : prev_real_y2;

        if (actual_real_y1 > 1 || actual_real_y2 < -1) {
            continue;
        }

        // Я знаю, как избавиться от всех операций деления в этой функции, но пока оставлю так.
        // Интерполяция аттрибутов
        // --------------------
        // x, y, z координаты
        Vector3 interpolated_point1 = lowest + (highest - lowest) * left_coeff;
        Vector3 interpolated_point2 = middle + (highest - middle) * right_coeff;
        // нормали
        // Здесь интерполирую через y, потому что такой способ убирает визуальные баги
        Vector3 interpolated_normal1 = highest_proj(1) == lowest_proj(1)
                                           ? highest_norm
                                           : lowest_norm + (highest_norm - lowest_norm) *
                                                               (actual_real_y1 - lowest_proj(1)) /
                                                               (highest_proj(1) - lowest_proj(1));
        Vector3 interpolated_normal2 = highest_proj(1) == middle_proj(1)
                                           ? highest_norm
                                           : middle_norm + (highest_norm - middle_norm) *
                                                               (actual_real_y2 - middle_proj(1)) /
                                                               (highest_proj(1) - middle_proj(1));
        // --------------------

        double real_z = lowest_proj(2) + (highest_proj(2) - lowest_proj(2)) * left_coeff;

        // Рисуем этот отрезок
        FillSegment(diffuse_color, specular_color, ambient, directional_lights, specular_pow, point_lights, spot_lights,
                    interpolated_point1, interpolated_point2, interpolated_normal1, interpolated_normal2,
                    actual_real_y1, actual_real_y2, x, real_z, z_diff_y, real_z_diff_y, frame, z_buffer_);
        prev_real_y1 = real_y1;
        prev_real_y2 = real_y2;
    }
    if (x + 1 < frame->Height()) {
        real_x = top_x;

        double left_coeff =
            highest_proj(0) == lowest_proj(0) ? 1 : (real_x - lowest_proj(0)) / (highest_proj(0) - lowest_proj(0));
        double right_coeff =
            highest_proj(0) == middle_proj(0) ? 1 : (real_x - middle_proj(0)) / (highest_proj(0) - middle_proj(0));

        double real_y1 = lowest_proj(1) + (highest_proj(1) - lowest_proj(1)) * left_coeff;
        double real_y2 = middle_proj(1) + (highest_proj(1) - middle_proj(1)) * right_coeff;

        double actual_real_y1 = real_y1 < prev_real_y1 ? real_y1 : prev_real_y1;
        double actual_real_y2 = real_y2 > prev_real_y2 ? real_y2 : prev_real_y2;

        if (actual_real_y1 > 1 || actual_real_y2 < -1) {
            return;
        }

        // Я знаю, как избавиться от всех операций деления в этой функции, но пока оставлю так.
        // Интерполяция аттрибутов
        // --------------------
        // x, y, z координаты
        Vector3 interpolated_point1 = lowest + (highest - lowest) * left_coeff;
        Vector3 interpolated_point2 = middle + (highest - middle) * right_coeff;
        // нормали
        // Здесь интерполирую через y, потому что такой способ убирает визуальные баги
        Vector3 interpolated_normal1 = highest_proj(1) == lowest_proj(1)
                                           ? highest_norm
                                           : lowest_norm + (highest_norm - lowest_norm) *
                                                               (actual_real_y1 - lowest_proj(1)) /
                                                               (highest_proj(1) - lowest_proj(1));
        Vector3 interpolated_normal2 = highest_proj(1) == middle_proj(1)
                                           ? highest_norm
                                           : middle_norm + (highest_norm - middle_norm) *
                                                               (actual_real_y2 - middle_proj(1)) /
                                                               (highest_proj(1) - middle_proj(1));
        // --------------------

        // Тут на всякий реально нужна эта прверка
        if (actual_real_y1 > actual_real_y2) {
            return;
        }

        double real_z = lowest_proj(2) + (highest_proj(2) - lowest_proj(2)) * left_coeff;

        // Рисуем этот отрезок
        FillSegment(diffuse_color, specular_color, ambient, directional_lights, specular_pow, point_lights, spot_lights,
                    interpolated_point1, interpolated_point2, interpolated_normal1, interpolated_normal2,
                    actual_real_y1, actual_real_y2, x, real_z, z_diff_y, real_z_diff_y, frame, z_buffer_);
    }
}

void DrawTriangle(const Triangle& triangle, const TriMatrix& projected_vertices,
                  const std::vector<PLSInSpace>& point_lights, const std::vector<SLSInSpace>& spot_lights,
                  const Color& ambient, const std::vector<DirectionalLightSource>& directional_lights, Frame* frame,
                  ZBuffer* z_buffer_) {
    // вершины треугольника, отсортированные по Ox.
    auto [lowest_proj, middle_proj, highest_proj, lowest, middle, highest, lowest_norm, middle_norm, highest_norm] =
        GetVerticalOrderOfVerticesAndAttributes(projected_vertices, triangle);

    // Костыль
    // --------------------
    // constexpr static double kEpsCorrection = 0.0016;
    // lowest_proj(0) -= kEpsCorrection;
    // highest_proj(0) += kEpsCorrection;
    // middle_proj(1) +=
    //     (CalcVecProdXY(highest_proj - lowest_proj, middle_proj - lowest_proj) > 0 ? kEpsCorrection :
    //     -kEpsCorrection);
    // --------------------

    // Получаю векторы, напраленные из нижней (по Ox) точки треугольника в среднюю и верхнюю соотв.
    Vector3 v1 = middle_proj - lowest_proj;
    Vector3 v2 = highest_proj - lowest_proj;
    // Получаю вектор нормали к плоскости треугольника.
    v1 = v1.cross(v2);

    // Смысл происходящего далее такой: я поддерживаю две точки, соответствующие друг другу, точку на треугольнике в
    // нашем видимом пространстве и точку треугольника на экране. Я двигаю точку на экране по оси Ox и Oy, при этом в
    // нужной пропорции изменяя координату точки в видимом пространстве с помощью посчитанных здесь коэффициентов.
    // real_z_diff_y -- Коэффициент изменения координаты z при изменении координаты y (т.е. такое, чтобы снова попасть в
    // плоскость треугольника) в видимом пространстве.
    // z_diff_y -- Коэффициент изменения координаты z при изменении координаты y на такое значение, чтобы кордината y на
    // экране сдвинулась на один пиксель.
    // real_y, real_z -- координаты поддреживаемой точки в видимом пространстве.
    double real_z_diff_y = (v1(2) == 0 ? 0 : -v1(1) / v1(2));
    double z_diff_y = real_z_diff_y * kSideOfTheCube / frame->Width();

    // Я передаю эти четыре переменные по указателю как изменяемые входные данные. Я мб позже сменю это на return tuple
    // или struct, мне нужно подумать.
    double real_x = (lowest_proj(0) >= -1 ? lowest_proj(0) : -1);
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

    auto [prev_real_y1, prev_real_y2, real_x2, x2] = FillLowerTriangle(
        triangle.diffuse_reflection_color, triangle.specular_reflection_color, ambient, directional_lights,
        triangle.specular_power, point_lights, spot_lights, lowest_proj, middle_proj, highest_proj, lowest, middle,
        highest, lowest_norm, middle_norm, highest_norm, real_z_diff_y, z_diff_y, real_x, x, frame, z_buffer_);

    FillUpperTriangle(triangle.diffuse_reflection_color, triangle.specular_reflection_color, ambient,
                      directional_lights, triangle.specular_power, point_lights, spot_lights, lowest_proj, middle_proj,
                      highest_proj, lowest, middle, highest, lowest_norm, middle_norm, highest_norm, real_z_diff_y,
                      z_diff_y, prev_real_y1, prev_real_y2, real_x2, x2, frame, z_buffer_);
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

Frame BufferRasterizer::MakeFrame(const std::vector<Triangle>& triangles, const std::vector<PLSInSpace>& point_lights,
                                  const std::vector<SLSInSpace>& spot_lights, const Color& ambient,
                                  const std::vector<DirectionalLightSource>& directional_lights, const Camera& camera,
                                  Frame&& frame) {
    Frame ret(std::move(frame));
    z_buffer_.FitTo(ret);
    ret.FillWithBlackColor();
    for (const Triangle& triangle : triangles) {
        // В целях оптимизации проективное преобразование пришлось перенести сюда.
        TriMatrix projected_vertices = ApplyFrustumTransformationOnTriangle(triangle, camera);
        DrawTriangle(triangle, projected_vertices, point_lights, spot_lights, ambient, directional_lights, &ret,
                     &z_buffer_);
    }
    return ret;
}

}  // namespace Renderer3D::Kernel
