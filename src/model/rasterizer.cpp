#include "rasterizer.h"

#include <cassert>

namespace Renderer3D::Kernel {
namespace {

constexpr double kSideOfTheCube = 2.0;

using VectorRef = const Eigen::Block<const Eigen::Matrix<double, 4, 3>, 4, 1, true>;

auto GetVerticalOrderOfVertices(const Triangle& triangle) {
    std::array<uint8_t, 3> ret = {0, 1, 2};

    if (triangle.vertices(0, ret[2]) < triangle.vertices(0, ret[1])) {
        std::swap(ret[2], ret[1]);
    }
    if (triangle.vertices(0, ret[2]) < triangle.vertices(0, ret[0])) {
        std::swap(ret[2], ret[0]);
    }
    if (triangle.vertices(0, ret[1]) < triangle.vertices(0, ret[0])) {
        std::swap(ret[1], ret[0]);
    }

    return std::array<VectorRef, 3>{triangle.vertices.col(ret[0]), triangle.vertices.col(ret[1]),
                                    triangle.vertices.col(ret[2])};
}

void FillSegment(const Triangle& triangle, Frame& frame, std::vector<double>& z_buffer_, ssize_t x, double real_y1,
                 double real_y2, double real_z, double z_diff_y, double real_z_diff_y) {

    ssize_t edge = frame.GetWidth() - 1;
    assert(edge >= 0);

    if (real_y1 < -1) {
        real_z += (-real_y1 - 1) * real_z_diff_y;
        real_y1 = -1;
    }

    // Есть непонятка, нужно обсудить.
    ssize_t y1 = frame.CalcYDiscreteFromRealSegment(real_y1, kSideOfTheCube);
    ssize_t y2 = frame.CalcYDiscreteFromRealSegment(real_y2, kSideOfTheCube);
    assert(y1 >= 0);
    assert(y2 >= 0);

    for (ssize_t y = y1; y <= (y2 >= edge ? edge : y2); ++y, real_z += z_diff_y) {
        if (real_z < z_buffer_[x * frame.GetWidth() + y]) {
            z_buffer_[x * frame.GetWidth() + y] = real_z;
            frame(x, y) = triangle.color;
        }
    }
}

void FillLowerTriangle(const Triangle& triangle, Frame& frame, std::vector<double>& z_buffer_, VectorRef lowest,
                       VectorRef middle, VectorRef highest, double real_z_diff_y, double z_diff_y, double z_diff_x,
                       double& real_x, double& real_z, ssize_t& x, double& prev_y) {
    double mid_x = (middle(0) <= 1 ? middle(0) : 1);
    double dx = kSideOfTheCube / frame.GetHeight();
    for (; real_x < mid_x; ++x, real_x += dx, real_z += z_diff_x) {
        // real_y1, real_y2 -- y координаты отрезка в видимом пространстве, который будет нарисован на экране.
        double real_y1 = (highest(0) == lowest(0)
                              ? highest(1)
                              : lowest(1) + (highest(1) - lowest(1)) * (real_x - lowest(0)) / (highest(0) - lowest(0)));
        double real_y2 = lowest(0) == middle(0)
                             ? middle(1)
                             : (lowest(1) + (middle(1) - lowest(1)) * (real_x - lowest(0)) / (middle(0) - lowest(0)));

        if (real_y1 > real_y2) {
            std::swap(real_y1, real_y2);
        }
        if (real_y2 < -1) {
            continue;
        }

        real_z += real_z_diff_y * (real_y1 - prev_y);
        prev_y = real_y1;

        // Рисуем этот отрезок
        FillSegment(triangle, frame, z_buffer_, x, real_y1, real_y2, real_z, z_diff_y, real_z_diff_y);
    }
}

void FillUpperTriangle(const Triangle& triangle, Frame& frame, std::vector<double>& z_buffer_, VectorRef lowest,
                       VectorRef middle, VectorRef highest, double real_z_diff_y, double z_diff_y, double z_diff_x,
                       double& real_x, double& real_z, ssize_t& x, double& prev_y) {
    double top_x = (highest(0) <= 1 ? highest(0) : 1);
    double dx = kSideOfTheCube / frame.GetHeight();
    for (; real_x < top_x; ++x, real_x += dx, real_z += z_diff_x) {
        // real_y1, real_y2 -- y координаты отрезка в видимом пространстве, который будет нарисован на экране.
        double real_y1 = (highest(0) == lowest(0)
                              ? highest(1)
                              : lowest(1) + (highest(1) - lowest(1)) * (real_x - lowest(0)) / (highest(0) - lowest(0)));
        double real_y2 =
            (highest(0) == middle(0)
                 ? highest(1)
                 : (middle(1) + (highest(1) - middle(1)) * (real_x - middle(0)) / (highest(0) - middle(0))));
        if (real_y1 > real_y2) {
            std::swap(real_y1, real_y2);
        }
        if (real_y2 < -1) {
            continue;
        }

        real_z += real_z_diff_y * (real_y1 - prev_y);
        prev_y = real_y1;

        // Рисуем этот отрезок
        FillSegment(triangle, frame, z_buffer_, x, real_y1, real_y2, real_z, z_diff_y, real_z_diff_y);
    }
}

void DrawTriangle(Frame& frame, std::vector<double>& z_buffer_, const Triangle& triangle) {

    // Eigen::Block'и вершин треугольника, отсортированные по Ox.
    auto [lowest, middle, highest] = GetVerticalOrderOfVertices(triangle);

    // Получаю векторы, напраленные из нижней (по Ox) точки треугольника в среднюю и верхнюю соотв.
    Vector3 v1 = middle.head(3) - lowest.head(3);
    Vector3 v2 = highest.head(3) - lowest.head(3);
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
    double z_diff_y = real_z_diff_y * kSideOfTheCube / frame.GetWidth();
    double z_diff_x = (v1(2) == 0 ? 0 : -v1(0) / v1(2) * kSideOfTheCube / frame.GetHeight());

    double real_x = (lowest(0) >= -1 ? lowest(0) : -1);
    double prev_y = lowest(1);

    double real_z = lowest(2) - (v1(2) == 0 ? 0 : v1(0) / v1(2)) * (real_x - lowest(0));

    ssize_t x = frame.CalcXDiscreteFromRealSegment(real_x, kSideOfTheCube);
    assert(x >= 0);

    // Треугольник разбивается на два других с одной из сторон параллельной Oy. Далее отриссовываем эти треугольники на
    // экране соотв. функциями.
    FillLowerTriangle(triangle, frame, z_buffer_, lowest, middle, highest, real_z_diff_y, z_diff_y, z_diff_x, real_x,
                      real_z, x, prev_y);

    FillUpperTriangle(triangle, frame, z_buffer_, lowest, middle, highest, real_z_diff_y, z_diff_y, z_diff_x, real_x,
                      real_z, x, prev_y);
}

}  // namespace

Frame BufferRasterizer::MakeFrame(const std::vector<Triangle>& triangles, Frame&& frame) {
    z_buffer_.assign(frame.GetHeight() * frame.GetWidth(), std::numeric_limits<double>::infinity());
    Frame ret(std::move(frame));
    ret.FillWithBlackColor();
    for (const auto& triangle : triangles) {
        DrawTriangle(ret, z_buffer_, triangle);
    }
    return ret;
}

}  // namespace Renderer3D::Kernel
