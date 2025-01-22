#pragma once
#include <Eigen/Dense>

namespace Renderer3D::Kernel {

class Camera {
    using Matrix4d = Eigen::Matrix4d;
public:
    Camera(double fov, double near_dist, double far_dist, double ratio);

    [[nodiscard]] double GetRatio() const;

    [[nodiscard]] double GetNearDist() const;

    [[nodiscard]] double GetFarDist() const;

    [[nodiscard]] double GetLeftIntersectX() const;

    [[nodiscard]] double GetRightIntersectX() const;

    [[nodiscard]] double GetTopIntersectY() const;

    [[nodiscard]] double GetBottomIntersectY() const;

    [[nodiscard]] const Matrix4d& GetFrustrum() const;

private:
    Matrix4d frustrum_;
    double fov_;
    double near_dist_;
    double far_dist_;
    double ratio_;
    double left_intersect_x_;
    double right_intersect_x_;
    double top_intersect_y_;
    double bottom_intersect_y_;
};

}  // namespace Renderer3D::Kernel
