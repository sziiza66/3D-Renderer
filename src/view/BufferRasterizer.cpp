#include "BufferRasterizer.h"

namespace Renderer3D::Kernel {
namespace {

inline void SetPixel(sf::Uint8* frame, const sf::Uint8* color, size_t x, size_t y, size_t width) {
    frame[(x * width + y) * 4] = color[0];
    frame[(x * width + y) * 4 + 1] = color[1];
    frame[(x * width + y) * 4 + 2] = color[2];
    frame[(x * width + y) * 4 + 3] = color[3];
}

inline bool IsTriangleInTheFrustrum(const Renderer3D::Kernel::Triangle& triangle) {
    return abs(triangle(2, 0)) < 1 && abs(triangle(2, 1)) < 1 && abs(triangle(2, 2)) < 1;
}

void DrawTriangle(Frame& frame, std::vector<double>& z_buffer_, const Renderer3D::Kernel::Triangle& triangle,
                  Color color) {
    if (!IsTriangleInTheFrustrum(triangle)) {
        return;
    }
    uint8_t lowest = 0;
    uint8_t middle = 1;
    uint8_t highest = 2;
    if (triangle(0, highest) < triangle(0, middle)) {
        std::swap(highest, middle);
    }
    if (triangle(0, highest) < triangle(0, lowest)) {
        std::swap(highest, lowest);
    }
    if (triangle(0, middle) < triangle(0, lowest)) {
        std::swap(middle, lowest);
    }

    Eigen::Vector3d v1;
    Eigen::Vector3d v2;
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
                SetPixel(frame, color, x, y, frame.GetWidth());
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
                SetPixel(frame, color, x, y, frame.GetWidth());
            }
        }
    }
}

}  // namespace

Frame BufferRasterizer::MakeFrame(size_t height, size_t width,
                                  const std::vector<std::pair<entity::Property, Triangle>>& triangles) {
    std::vector<double> z_buffer(height * width, std::numeric_limits<double>::infinity());
    Frame frame(height, width);
    for (const auto& pair : triangles) {
        DrawTriangle(frame, z_buffer, pair.second, pair.first.GetColor());
    }
    return frame;
}

}  // namespace Renderer3D::Kernel
