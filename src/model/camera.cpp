#include "camera.h"

namespace Renderer3D::Kernel {

namespace {

Matrix4 CalculateFrustumMatrix(double fov, double near_dist, double ratio) {
    assert(fov > 0);
    assert(fov < std::numbers::pi);
    assert(near_dist > 0);
    assert(!std::isinf(near_dist));
    assert(ratio > 0);
    assert(!std::isinf(ratio));

    double proj_dist = 1.0 / std::tan(fov / 2);
    double proj_dist2 = proj_dist * proj_dist;
    double ratio2 = ratio * ratio;

    double left_intersect_x_ = -near_dist / proj_dist;
    double right_intersect_x_ = near_dist / proj_dist;
    double top_intersect_y_ = ratio * near_dist / proj_dist;
    double bottom_intersect_y_ = -ratio * near_dist / proj_dist;
    return Matrix4{{near_dist * 2 / (right_intersect_x_ - left_intersect_x_), 0,
                    (right_intersect_x_ + left_intersect_x_) / (right_intersect_x_ - left_intersect_x_), 0},
                   {0, near_dist * 2 / (top_intersect_y_ - bottom_intersect_y_),
                    (top_intersect_y_ + bottom_intersect_y_) / (top_intersect_y_ - bottom_intersect_y_), 0},
                   {0, 0, -1, near_dist * 2},
                   {0, 0, -1, 0}};
    ;
}

}  // namespace

Camera::Camera(double ratio)
    : frustum_matrix_(CalculateFrustumMatrix(kDefaultFov, kDefaultNearPlaneDistance, ratio)),
      fov_(kDefaultFov),
      near_dist_(kDefaultNearPlaneDistance),
      ratio_(ratio) {
}

Camera::Camera(double fov, double near_dist, double ratio)
    : frustum_matrix_(CalculateFrustumMatrix(fov, near_dist, ratio)), fov_(fov), near_dist_(near_dist), ratio_(ratio) {
}

double Camera::Ratio() const {
    return ratio_;
}

double Camera::NearDistance() const {
    return near_dist_;
}

const Matrix4& Camera::FrustumMatrix() const {
    return frustum_matrix_;
}

}  // namespace Renderer3D::Kernel
