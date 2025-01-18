#include "triangle.h"

namespace Renderer3D::Kernel {

void Triangle::NormalizeFourthCoordinate() {
    matrix_(3, 0) = 1;
    matrix_(3, 1) = 1;
    matrix_(3, 2) = 1;
}

Triangle::Triangle() : matrix_(kDEFAULTTRIANGLE) {
}

Triangle::Triangle(const Eigen::Matrix<double, 4, 3>& matrix) : matrix_(matrix) {
    NormalizeFourthCoordinate();
}

double Triangle::operator()(long x, long y) const {
    return matrix_(x, y);
}

Eigen::Vector4d Triangle::operator()(long x) const {
    return matrix_.col(x);
}

const Eigen::Matrix<double, 4, 3>& Triangle::GetPointsMatrix() const {
    return matrix_;
}

const Eigen::Matrix<double, 4, 3> Triangle::kDEFAULTTRIANGLE{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {1, 1, 1}};

}  // namespace Renderer3D::Kernel
