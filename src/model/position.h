#pragma once

#include <Eigen/Dense>

namespace Renderer3D::Kernel {

class Position {
public:
    Position();

    Position(Eigen::Matrix3d orthogonal, Eigen::Vector3d shift);

private:
    Eigen::Matrix4d matrix_;
};

}  // namespace Renderer3D::Kernel
