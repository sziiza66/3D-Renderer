#pragma once

#include <Eigen/Dense>

namespace Renderer3D::Kernel {

class Position {
    using Matrix4d = Eigen::Matrix4d;
    using Matrix3d = Eigen::Matrix3d;
    using Vector3d = Eigen::Vector3d;

public:
    Position();

    Position(Matrix3d orthogonal, Vector3d shift);

    Position& operator*=(const Position& other);

    [[nodiscard]] const Matrix4d& GetMatrix() const;

    [[nodiscard]] Position Inverse() const;

private:
    Matrix4d matrix_;
};

Position operator*(const Position& lhs, const Position& rhs);

}  // namespace Renderer3D::Kernel
