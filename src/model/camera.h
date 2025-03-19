#pragma once

#include "../aliases.h"

namespace Renderer3D::Kernel {

class Camera {

public:
    Camera(double fov, double near_dist, double far_dist, double ratio);

    [[nodiscard]] double GetRatio() const;
    [[nodiscard]] double GetNearDist() const;
    [[nodiscard]] double GetLeftIntersectX() const;
    [[nodiscard]] double GetRightIntersectX() const;
    [[nodiscard]] double GetTopIntersectY() const;
    [[nodiscard]] double GetBottomIntersectY() const;
    [[nodiscard]] const Matrix4& GetFrustumMatrix() const;

private:
    Matrix4 frustum_matrix_;
    double fov_;
    double near_dist_;
    double ratio_;
    double left_intersect_x_;
    double right_intersect_x_;
    double top_intersect_y_;
    double bottom_intersect_y_;
};

}  // namespace Renderer3D::Kernel
