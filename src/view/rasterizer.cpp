#include "rasterizer.h"

static inline void SetPixel(sf::Uint8* frame, const sf::Uint8* color, size_t x, size_t y, size_t width) {
    frame[(x * width + y) * 4] = color[0];
    frame[(x * width + y) * 4 + 1] = color[1];
    frame[(x * width + y) * 4 + 2] = color[2];
    frame[(x * width + y) * 4 + 3] = color[3];
}

void view::DotRasterizer::SetResolution(size_t width, size_t height) {
    height_ = height;
    width_ = width;
}

void view::DotRasterizer::operator()(sf::Uint8* frame, const entity::Triangle& triangle, const sf::Uint8* color) const {
    for (size_t i = 0; i < 3; ++i) {
        if (triangle(0, i) > -1 && triangle(0, i) < 1 && triangle(1, i) > -1 && triangle(1, i) < 1 &&
            triangle(2, i) > 0) {
            size_t x = (triangle(0, i) + 1) / 2 * height_;
            size_t y = (triangle(1, i) + 1) / 2 * width_;
            SetPixel(frame, color, x, y, width_);
        }
    }
}

void view::NoBufferRasterizer::SetResolution(size_t width, size_t height) {
    height_ = height;
    width_ = width;
}

void view::NoBufferRasterizer::operator()(sf::Uint8* frame, const entity::Triangle& triangle,
                                          const sf::Uint8* color) const {
    if (abs(triangle(2, 0)) >= 1 || abs(triangle(2, 1)) >= 1 || abs(triangle(2, 2)) >= 1) {
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

    double real_x = (triangle(0, lowest) >= -1 ? triangle(0, lowest) : -1);
    double mid_x = (triangle(0, middle) <= 1 ? triangle(0, middle) : 1);
    double top_x = (triangle(0, highest) <= 1 ? triangle(0, highest) : 1);
    size_t x = (real_x + 1) / 2 * height_;
    for (; real_x < mid_x; ++x, real_x += 2.0 / height_) {
        int64_t y1 = (triangle(1, lowest) +
                      (triangle(1, highest) - triangle(1, lowest)) * (real_x - triangle(0, lowest)) /
                          (triangle(0, highest) - triangle(0, lowest)) +
                      1) /
                     2 * width_;
        int64_t y2 = (triangle(0, lowest) == triangle(0, middle)
                          ? (triangle(1, middle) + 1) / 2 * width_
                          : (triangle(1, lowest) +
                             (triangle(1, middle) - triangle(1, lowest)) * (real_x - triangle(0, lowest)) /
                                 (triangle(0, middle) - triangle(0, lowest)) +
                             1) /
                                2 * width_);
        if (y1 > y2) {
            std::swap(y1, y2);
        }
        int64_t edge = width_ - 1;
        for (int64_t y = (y1 < 0 ? 0 : y1); y <= (y2 >= edge ? edge : y2); ++y) {

            SetPixel(frame, color, x, y, width_);
        }
    }
    for (; real_x < top_x; ++x, real_x += 2.0 / height_) {
        int64_t y1 = (triangle(1, lowest) +
                      (triangle(1, highest) - triangle(1, lowest)) * (real_x - triangle(0, lowest)) /
                          (triangle(0, highest) - triangle(0, lowest)) +
                      1) /
                     2 * width_;
        int64_t y2 = (triangle(0, highest) == triangle(0, middle)
                          ? (triangle(1, highest) + 1) / 2 * width_
                          : (triangle(1, middle) +
                             (triangle(1, highest) - triangle(1, middle)) * (real_x - triangle(0, middle)) /
                                 (triangle(0, highest) - triangle(0, middle)) +
                             1) /
                                2 * width_);

        if (y1 > y2) {
            std::swap(y1, y2);
        }
        int64_t edge = width_ - 1;
        for (int64_t y = (y1 < 0 ? 0 : y1); y <= (y2 >= edge ? edge : y2); ++y) {
            SetPixel(frame, color, x, y, width_);
        }
    }
}

void view::BufferRasterizer::SetResolution(size_t width, size_t height) {
    delete[] z_buffer_;
    height_ = height;
    width_ = width;
    z_buffer_ = new double[width * height];
}

void view::BufferRasterizer::operator()(sf::Uint8* frame, const entity::Triangle& triangle,
                                        const sf::Uint8* color) const {
    if (abs(triangle(2, 0)) >= 1 || abs(triangle(2, 1)) >= 1 || abs(triangle(2, 2)) >= 1) {
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
    double z_diff_y = real_z_diff_y * 2 / width_;
    double z_diff_x = (v1(2) == 0 ? 0 : -v1(0) / v1(2) * 2 / height_);

    double real_x = (triangle(0, lowest) >= -1 ? triangle(0, lowest) : -1);
    double real_z = triangle(2, lowest) - v1(0) / v1(2) * (real_x - triangle(0, lowest));
    double prev_y = triangle(1, lowest);

    double mid_x = (triangle(0, middle) <= 1 ? triangle(0, middle) : 1);
    double top_x = (triangle(0, highest) <= 1 ? triangle(0, highest) : 1);
    size_t x = (real_x + 1) / 2 * height_;
    for (; real_x < mid_x; ++x, real_x += 2.0 / height_, real_z += z_diff_x) {
        double real_y1 = triangle(1, lowest) + (triangle(1, highest) - triangle(1, lowest)) *
                                                   (real_x - triangle(0, lowest)) /
                                                   (triangle(0, highest) - triangle(0, lowest));
        int64_t y1 = (real_y1 + 1) / 2 * width_;
        double real_y2 =
            triangle(0, lowest) == triangle(0, middle)
                ? triangle(1, middle)
                : (triangle(1, lowest) + (triangle(1, middle) - triangle(1, lowest)) * (real_x - triangle(0, lowest)) /
                                             (triangle(0, middle) - triangle(0, lowest)));
        int64_t y2 = (real_y2 + 1) / 2 * width_;

        if (real_y1 > real_y2) {
            std::swap(y1, y2);
            std::swap(real_y1, real_y2);
        }
        real_z += real_z_diff_y * (real_y1 - prev_y);
        prev_y = real_y1;

        double real_z_cpy = real_z;
        int64_t edge = width_ - 1;
        for (int64_t y = (y1 < 0 ? 0 : y1); y <= (y2 >= edge ? edge : y2); ++y, real_z_cpy += z_diff_y) {
            if (real_z_cpy < z_buffer_[x * width_ + y]) {
                z_buffer_[x * width_ + y] = real_z_cpy;
                SetPixel(frame, color, x, y, width_);
            }
        }
    }
    for (; real_x < top_x; ++x, real_x += 2.0 / height_, real_z += z_diff_x) {
        double real_y1 = triangle(1, lowest) + (triangle(1, highest) - triangle(1, lowest)) *
                                                   (real_x - triangle(0, lowest)) /
                                                   (triangle(0, highest) - triangle(0, lowest));
        int64_t y1 = (real_y1 + 1) / 2 * width_;
        double real_y2 = (triangle(0, highest) == triangle(0, middle)
                              ? triangle(1, highest)
                              : (triangle(1, middle) + (triangle(1, highest) - triangle(1, middle)) *
                                                           (real_x - triangle(0, middle)) /
                                                           (triangle(0, highest) - triangle(0, middle))));
        int64_t y2 = (real_y2 + 1) / 2 * width_;

        if (real_y1 > real_y2) {
            std::swap(y1, y2);
            std::swap(real_y1, real_y2);
        }
        real_z += real_z_diff_y * (real_y1 - prev_y);
        prev_y = real_y1;

        double real_z_cpy = real_z;
        int64_t edge = width_ - 1;
        for (int64_t y = (y1 < 0 ? 0 : y1); y <= (y2 >= edge ? edge : y2); ++y, real_z_cpy += z_diff_y) {
            if (real_z_cpy < z_buffer_[x * width_ + y]) {
                z_buffer_[x * width_ + y] = real_z_cpy;
                SetPixel(frame, color, x, y, width_);
            }
        }
    }
}
