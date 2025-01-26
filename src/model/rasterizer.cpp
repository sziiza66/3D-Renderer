#include "rasterizer.h"

#include <tuple>

namespace Renderer3D::Kernel {
namespace {

bool IsTriangleInTheFrustrum(const Renderer3D::Kernel::Triangle& triangle) {
    return abs(triangle(2, 0)) < 1 && abs(triangle(2, 1)) < 1 && abs(triangle(2, 2)) < 1;
}

std::tuple<uint8_t, uint8_t, uint8_t> GetVerticalOrderOfVertices(const Renderer3D::Kernel::Triangle& triangle) {
    std::tuple<uint8_t, uint8_t, uint8_t> ret = {0, 1, 2};
    if (triangle(0, std::get<2>(ret)) < triangle(0, std::get<1>(ret))) {
        std::swap(std::get<2>(ret), std::get<1>(ret));
    }
    if (triangle(0, std::get<2>(ret)) < triangle(0, std::get<0>(ret))) {
        std::swap(std::get<2>(ret), std::get<0>(ret));
    }
    if (triangle(0, std::get<1>(ret)) < triangle(0, std::get<0>(ret))) {
        std::swap(std::get<1>(ret), std::get<0>(ret));
    }
    return ret;
}

inline void FillLine(const Triangle& triangle, Frame& frame, std::vector<double>& z_buffer_, size_t x, size_t y1,
                     size_t y2, double real_z, double z_diff_y) {
    size_t edge = frame.GetWidth() - 1;
    for (size_t y = (y1 < 0 ? 0 : y1); y <= (y2 >= edge ? edge : y2); ++y, real_z += z_diff_y) {
        if (real_z < z_buffer_[x * frame.GetWidth() + y]) {
            z_buffer_[x * frame.GetWidth() + y] = real_z;
            frame(x, y) = triangle.GetColor();
        }
    }
}

inline void FillLowerTriangle(const Triangle& triangle, Frame& frame, std::vector<double>& z_buffer_, uint8_t lowest,
                              uint8_t middle, uint8_t highest, double real_z_diff_y, double z_diff_y, double z_diff_x,
                              double& real_x, double& real_z, size_t& x, double& prev_y) {
    double mid_x = (triangle(0, middle) <= 1 ? triangle(0, middle) : 1);
    double dx = 2.0 / frame.GetHeight();
    for (; real_x < mid_x; ++x, real_x += dx, real_z += z_diff_x) {
        double real_y1 = triangle(1, lowest) + (triangle(1, highest) - triangle(1, lowest)) *
                                                   (real_x - triangle(0, lowest)) /
                                                   (triangle(0, highest) - triangle(0, lowest));
        size_t y1 = ((real_y1 < -1 ? -1 : real_y1) + 1) / 2 * frame.GetWidth();
        double real_y2 =
            triangle(0, lowest) == triangle(0, middle)
                ? triangle(1, middle)
                : (triangle(1, lowest) + (triangle(1, middle) - triangle(1, lowest)) * (real_x - triangle(0, lowest)) /
                                             (triangle(0, middle) - triangle(0, lowest)));
        size_t y2 = ((real_y2 < -1 ? -1 : real_y2) + 1) / 2 * frame.GetWidth();

        if (real_y1 > real_y2) {
            std::swap(y1, y2);
            std::swap(real_y1, real_y2);
        }
        real_z += real_z_diff_y * (real_y1 - prev_y);
        prev_y = real_y1;

        FillLine(triangle, frame, z_buffer_, x, y1, y2, real_z, z_diff_y);
    }
}

inline void FillUpperTriangle(const Triangle& triangle, Frame& frame, std::vector<double>& z_buffer_, uint8_t lowest,
                              uint8_t middle, uint8_t highest, double real_z_diff_y, double z_diff_y, double z_diff_x,
                              double& real_x, double& real_z, size_t& x, double& prev_y) {
    double top_x = (triangle(0, highest) <= 1 ? triangle(0, highest) : 1);
    for (; real_x < top_x; ++x, real_x += 2.0 / frame.GetHeight(), real_z += z_diff_x) {
        double real_y1 = triangle(1, lowest) + (triangle(1, highest) - triangle(1, lowest)) *
                                                   (real_x - triangle(0, lowest)) /
                                                   (triangle(0, highest) - triangle(0, lowest));
        size_t y1 = ((real_y1 < -1 ? -1 : real_y1) + 1) / 2 * frame.GetWidth();
        double real_y2 = (triangle(0, highest) == triangle(0, middle)
                              ? triangle(1, highest)
                              : (triangle(1, middle) + (triangle(1, highest) - triangle(1, middle)) *
                                                           (real_x - triangle(0, middle)) /
                                                           (triangle(0, highest) - triangle(0, middle))));
        size_t y2 = ((real_y2 < -1 ? -1 : real_y2) + 1) / 2 * frame.GetWidth();

        if (real_y1 > real_y2) {
            std::swap(y1, y2);
            std::swap(real_y1, real_y2);
        }
        real_z += real_z_diff_y * (real_y1 - prev_y);
        prev_y = real_y1;

        FillLine(triangle, frame, z_buffer_, x, y1, y2, real_z, z_diff_y);
    }
}

void DrawTriangle(Frame& frame, std::vector<double>& z_buffer_, const Triangle& triangle) {
    if (!IsTriangleInTheFrustrum(triangle)) {
        return;
    }

    auto [lowest, middle, highest] = GetVerticalOrderOfVertices(triangle);

    // Получаю векторы, напраленные из нижней (по Ox) точки треугольника в среднюю и верхнюю соотв.
    Eigen::Vector3d v1 = triangle(middle).head(3) - triangle(lowest).head(3);
    Eigen::Vector3d v2 = triangle(highest).head(3) - triangle(lowest).head(3);
    // Получаю вектр нормали к плоскоссти треугольника.
    v1 = v1.cross(v2);

    // Смыссл происходящего далее такой: я поддерживаю две точки, соответствующие друг другу, точку на треугольнике в
    // нажем вимимом пространстве и точку треугольника на экране. Я двигаю точку на экране по оси Ox и Oy, при этом в
    // нужной пропорции изменяя координату точки в видимом пространстве с помощью посчитанных здесь коэффициентов.
    // real_z_diff_y -- Коэффициент изменения координаты z при изменении координаты y (т.е. такое, чтобы снова попасть в
    // плоскость треугольника) в видимом пространстве.
    // z_diff_y -- Коэффициент изменения координаты z при изменении координаты y на такое значение, чтобы кордината y на
    // экране сдвинулась на один пиксель.
    // z_diff_x -- аналогично для x.
    // real_x, real_y, real_z -- координаты пооддреживаемой точки в видимом прстранстве.

    double real_z_diff_y = (v1(2) == 0 ? 0 : -v1(1) / v1(2));
    double z_diff_y = real_z_diff_y * 2 / frame.GetWidth();
    double z_diff_x = (v1(2) == 0 ? 0 : -v1(0) / v1(2) * 2 / frame.GetHeight());

    double real_x = (triangle(0, lowest) >= -1 ? triangle(0, lowest) : -1);
    double prev_y = triangle(1, lowest);
    double real_z = triangle(2, lowest) - v1(0) / v1(2) * (real_x - triangle(0, lowest));

    size_t x = (real_x + 1) / 2 * frame.GetHeight();

    FillLowerTriangle(triangle, frame, z_buffer_, lowest, middle, highest, real_z_diff_y, z_diff_y, z_diff_x, real_x,
                      real_z, x, prev_y);

    FillUpperTriangle(triangle, frame, z_buffer_, lowest, middle, highest, real_z_diff_y, z_diff_y, z_diff_x, real_x,
                      real_z, x, prev_y);
}

}  // namespace

Frame BufferRasterizer::MakeFrame(const std::vector<Triangle>& triangles, Frame&& frame) {
    std::vector<double> z_buffer(frame.GetHeight() * frame.GetWidth(), std::numeric_limits<double>::infinity());
    Frame ret(std::move(frame));
    ret.Clear();
    for (const auto& triangle : triangles) {
        DrawTriangle(ret, z_buffer, triangle);
    }
    return ret;
}

}  // namespace Renderer3D::Kernel
