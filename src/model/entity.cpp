#include "entity.h"

#include <numbers>

entity::Property::Property(const Eigen::Matrix4d& pos, sf::Uint8 r, sf::Uint8 g, sf::Uint8 b) : pos_(pos) {
    color_[0] = r;
    color_[1] = g;
    color_[2] = b;
    color_[3] = 255;
}

entity::Property::Property(Eigen::Matrix4d&& pos, sf::Uint8 r, sf::Uint8 g, sf::Uint8 b) : pos_(std::move(pos)) {
    color_[0] = r;
    color_[1] = g;
    color_[2] = b;
    color_[3] = 255;
}

entity::Property::Property(const Property& other) : pos_(other.pos_) {
    for (size_t i = 0; i < 4; ++i) {
        color_[i] = other.color_[i];
    }
}

entity::Property::Property(Property&& other) : pos_(std::move(other.pos_)) {
    for (size_t i = 0; i < 4; ++i) {
        color_[i] = other.color_[i];
    }
}

entity::Camera::Camera(double fov, double near_dist, double far_dist, double ratio)
    : fov_(fov), near_dist_(near_dist), far_dist_(far_dist), ratio_(ratio) {
    assert(fov > 0);
    assert(fov < std::numbers::pi);
    assert(near_dist > 0);
    assert(far_dist > near_dist);
    assert(!std::isinf(far_dist));
    assert(!std::isinf(near_dist));
    assert(ratio > 0);
    assert(!std::isinf(ratio));

    double proj_dist = 1.0 / std::tan(fov / 2);
    double proj_dist2 = proj_dist * proj_dist;
    double ratio2 = ratio * ratio;

    near_ = {0, 0, -1, -near_dist};
    far_ = {0, 0, 1, far_dist};
    left_ = {proj_dist / std::sqrt(proj_dist2 + 1), 0, -1 / std::sqrt(proj_dist2 + 1), 0};
    right_ = {-proj_dist / std::sqrt(proj_dist2 + 1), 0, -1 / std::sqrt(proj_dist2 + 1), 0};
    bottom_ = {0, proj_dist / std::sqrt(proj_dist2 + ratio2), -ratio / std::sqrt(proj_dist2 + ratio2), 0};
    top_ = {0, -proj_dist / std::sqrt(proj_dist2 + ratio2), -ratio / std::sqrt(proj_dist2 + ratio2), 0};

    left_intersect_x_ = -near_dist_ / proj_dist;
    right_intersect_x_ = near_dist / proj_dist;
    top_intersect_y_ = ratio * near_dist / proj_dist;
    bottom_intersect_y_ = -ratio * near_dist / proj_dist;
}
