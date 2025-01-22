#include "position.h"

#include <stdexcept>

namespace Renderer3D::Kernel {

Position::Position() : matrix_(Matrix4d::Identity()) {
}

Position::Position(Matrix3d orthogonal, Vector3d shift) {
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

// Во время последовательного перемножения накапливается ошибка, я сделаю функцию коррекции позже, пока что это не
// кртитично.
Position& Position::operator*=(const Position& other) {
    matrix_ *= other.matrix_;
}

const Position::Matrix4d& Position::GetMatrix() const {
    return matrix_;
}

Position Position::Inverse() const {
    Position ret = *this;
    ret.matrix_ = ret.matrix_.inverse();
    return ret;
}

Position operator*(const Position& lhs, const Position& rhs) {
    Position ret = lhs;
    ret *= rhs;
    return ret;
}

}  // namespace Renderer3D::Kernel
