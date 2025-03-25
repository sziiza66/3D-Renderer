#pragma once

#include "../linalg.h"

namespace Renderer3D::Kernel {

class Camera {

public:
    Camera();

    Camera(double fov, double near_dist, double ratio);

    [[nodiscard]] double GetRatio() const;
    [[nodiscard]] double GetNearDist() const;
    [[nodiscard]] const Matrix4& GetFrustumMatrix() const;

private:
    Matrix4 frustum_matrix_;
    double fov_;
    double near_dist_;
    double ratio_;
};

}  // namespace Renderer3D::Kernel
