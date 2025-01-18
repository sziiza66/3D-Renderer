#include "position.h"

#include <stdexcept>

namespace Renderer3D::Kernel {

Position::Position() : matrix_(Eigen::Matrix4d::Identity()) {
}

Position::Position(Eigen::Matrix3d orthogonal, Eigen::Vector3d shift) {
    if (!orthogonal.isUnitary()) {
        throw std::invalid_argument("The motion matrix must be orthogonal when constructing Position.");
    }
    for (size_t i = 0; i < 3; ++i) {
        for (size_t j = 0; j < 3; ++j) {
            matrix_(i, j) = orthogonal(i, j);
        }
    }
    for (size_t i = 0; i < 3; ++i) {
        matrix_(i, 3) = shift(i);
    }
    matrix_(3, 3) = 1;
}

}  // namespace Renderer3D::Kernel
