#include "camera.h"

#include <cassert>
#include <cmath>
#include <numbers>

namespace Renderer3D::Kernel {

Camera::Camera(double fov, double near_dist, double far_dist, double ratio)
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

    left_intersect_x_ = -near_dist_ / proj_dist;
    right_intersect_x_ = near_dist / proj_dist;
    top_intersect_y_ = ratio * near_dist / proj_dist;
    bottom_intersect_y_ = -ratio * near_dist / proj_dist;

    frustum_ = Matrix4d{{near_dist_ * 2 / (right_intersect_x_ - left_intersect_x_), 0,
                         (right_intersect_x_ + left_intersect_x_) / (right_intersect_x_ - left_intersect_x_), 0},
                        {0, near_dist_ * 2 / (top_intersect_y_ - bottom_intersect_y_),
                         (top_intersect_y_ + bottom_intersect_y_) / (top_intersect_y_ - bottom_intersect_y_), 0},
                        {0, 0, -(far_dist_ + near_dist_) / (far_dist_ - near_dist_),
                         near_dist_ * far_dist_ * 2 / (far_dist_ - near_dist_)},
                        {0, 0, -1, 0}};
}

double Camera::GetRatio() const {
    return ratio_;
}

double Camera::GetNearDist() const {
    return near_dist_;
}

double Camera::GetFarDist() const {
    return far_dist_;
}

double Camera::GetLeftIntersectX() const {
    return left_intersect_x_;
}

double Camera::GetRightIntersectX() const {
    return right_intersect_x_;
}

double Camera::GetTopIntersectY() const {
    return top_intersect_y_;
}

double Camera::GetBottomIntersectY() const {
    return bottom_intersect_y_;
}

const Camera::Matrix4d& Camera::GetFrustum() const {
    return frustum_;
}

}  // namespace Renderer3D::Kernel
