#pragma once

#include "../linalg.h"

namespace Renderer3D::Kernel {

class Camera {

public:
    // Удаление дефолтного конструкторо мотивировано тем же, что и в случае class Spectator, см. комментарии там.
    Camera() = delete;
    explicit Camera(double ratio);
    Camera(double fov, double near_dist, double ratio);

    [[nodiscard]] double Ratio() const;
    [[nodiscard]] double NearDistance() const;
    [[nodiscard]] const Matrix4& FrustumMatrix() const;

private:
    static constexpr double kDefaultFov = std::numbers::pi / 2.1;
    static constexpr double kDefaultNearPlaneDistance = 0.1;

private:
    Matrix4 frustum_matrix_;
    double fov_;
    double near_dist_;
    double ratio_;
};

}  // namespace Renderer3D::Kernel
