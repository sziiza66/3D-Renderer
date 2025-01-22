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

void DrawTriangle(Frame& frame, std::vector<double>& z_buffer_, const Triangle& triangle) {
    if (!IsTriangleInTheFrustrum(triangle)) {
        return;
    }

    auto [lowest, middle, highest] = GetVerticalOrderOfVertices(triangle);

    Eigen::Vector3d v1;
    Eigen::Vector3d v2;
    // Eigen::Vector3d v1 = triangle(middle).head(3) - triangle(lowest).head(3);
    // Eigen::Vector3d v2 = triangle(highest).head(3) - triangle(lowest).head(3);
    for (size_t i = 0; i < 3; ++i) {
        v1(i) = triangle(i, middle) - triangle(i, lowest);
        v2(i) = triangle(i, highest) - triangle(i, lowest);
    }
    v1 = v1.cross(v2);
    double real_z_diff_y = (v1(2) == 0 ? 0 : -v1(1) / v1(2));
    double z_diff_y = real_z_diff_y * 2 / frame.GetWidth();
    double z_diff_x = (v1(2) == 0 ? 0 : -v1(0) / v1(2) * 2 / frame.GetHeight());

    double real_x = (triangle(0, lowest) >= -1 ? triangle(0, lowest) : -1);
    double real_z = triangle(2, lowest) - v1(0) / v1(2) * (real_x - triangle(0, lowest));
    double prev_y = triangle(1, lowest);

    double mid_x = (triangle(0, middle) <= 1 ? triangle(0, middle) : 1);
    double top_x = (triangle(0, highest) <= 1 ? triangle(0, highest) : 1);
    size_t x = (real_x + 1) / 2 * frame.GetHeight();
    for (; real_x < mid_x; ++x, real_x += 2.0 / frame.GetHeight(), real_z += z_diff_x) {
        double real_y1 = triangle(1, lowest) + (triangle(1, highest) - triangle(1, lowest)) *
                                                   (real_x - triangle(0, lowest)) /
                                                   (triangle(0, highest) - triangle(0, lowest));
        int64_t y1 = (real_y1 + 1) / 2 * frame.GetWidth();
        double real_y2 =
            triangle(0, lowest) == triangle(0, middle)
                ? triangle(1, middle)
                : (triangle(1, lowest) + (triangle(1, middle) - triangle(1, lowest)) * (real_x - triangle(0, lowest)) /
                                             (triangle(0, middle) - triangle(0, lowest)));
        int64_t y2 = (real_y2 + 1) / 2 * frame.GetWidth();

        if (real_y1 > real_y2) {
            std::swap(y1, y2);
            std::swap(real_y1, real_y2);
        }
        real_z += real_z_diff_y * (real_y1 - prev_y);
        prev_y = real_y1;

        double real_z_cpy = real_z;
        int64_t edge = frame.GetWidth() - 1;
        for (int64_t y = (y1 < 0 ? 0 : y1); y <= (y2 >= edge ? edge : y2); ++y, real_z_cpy += z_diff_y) {
            if (real_z_cpy < z_buffer_[x * frame.GetWidth() + y]) {
                z_buffer_[x * frame.GetWidth() + y] = real_z_cpy;
                frame(x, y) = triangle.GetColor();
            }
        }
    }
    for (; real_x < top_x; ++x, real_x += 2.0 / frame.GetHeight(), real_z += z_diff_x) {
        double real_y1 = triangle(1, lowest) + (triangle(1, highest) - triangle(1, lowest)) *
                                                   (real_x - triangle(0, lowest)) /
                                                   (triangle(0, highest) - triangle(0, lowest));
        int64_t y1 = (real_y1 + 1) / 2 * frame.GetWidth();
        double real_y2 = (triangle(0, highest) == triangle(0, middle)
                              ? triangle(1, highest)
                              : (triangle(1, middle) + (triangle(1, highest) - triangle(1, middle)) *
                                                           (real_x - triangle(0, middle)) /
                                                           (triangle(0, highest) - triangle(0, middle))));
        int64_t y2 = (real_y2 + 1) / 2 * frame.GetWidth();

        if (real_y1 > real_y2) {
            std::swap(y1, y2);
            std::swap(real_y1, real_y2);
        }
        real_z += real_z_diff_y * (real_y1 - prev_y);
        prev_y = real_y1;

        double real_z_cpy = real_z;
        int64_t edge = frame.GetWidth() - 1;
        for (int64_t y = (y1 < 0 ? 0 : y1); y <= (y2 >= edge ? edge : y2); ++y, real_z_cpy += z_diff_y) {
            if (real_z_cpy < z_buffer_[x * frame.GetWidth() + y]) {
                z_buffer_[x * frame.GetWidth() + y] = real_z_cpy;
                frame(x, y) = triangle.GetColor();
            }
        }
    }
}

}  // namespace

Frame BufferRasterizer::MakeFrame(size_t height, size_t width, const std::vector<Triangle>& triangles) {
    std::vector<double> z_buffer(height * width, std::numeric_limits<double>::infinity());
    Frame frame(height, width);
    for (const auto& triangle : triangles) {
        DrawTriangle(frame, z_buffer, triangle);
    }
    return frame;
}

}  // namespace Renderer3D::Kernel
