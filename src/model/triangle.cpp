#include "triangle.h"

namespace Renderer3D::Kernel {

void Triangle::NormalizeFourthCoordinate() {
    matrix_(3, 0) = 1;
    matrix_(3, 1) = 1;
    matrix_(3, 2) = 1;
}

Triangle::Triangle() : matrix_(DefaultCoordinates) {
}

Triangle::Triangle(Matrix matrix, Color color) : matrix_(std::move(matrix)), color_(color) {
    NormalizeFourthCoordinate();
}

double Triangle::operator()(long x, long y) const {
    return matrix_(x, y);
}

Eigen::Vector4d Triangle::operator()(long x) const {
    return matrix_.col(x);
}

const Triangle::Matrix& Triangle::GetPointsMatrix() const {
    return matrix_;
}

Color Triangle::GetColor() const {
    return color_;
}

void Triangle::ApplyPosition(const Position& pos) {
    matrix_ = pos.GetMatrix() * matrix_;
}

void Triangle::ApplyFrustrum(const Camera& cam) {
    matrix_ = cam.GetFrustrum() * matrix_;
    for (uint8_t j = 0; j < 3; ++j) {
        for (uint8_t i = 0; i < 3; ++i) {
            matrix_(j, i) /= matrix_(3, i);
        }
    }
    NormalizeFourthCoordinate();
}

const Triangle::Matrix Triangle::DefaultCoordinates{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {1, 1, 1}};

Triangle operator*(const Position& pos, const Triangle& triangle) {
    Triangle ret = triangle;
    ret.ApplyPosition(pos);
    return ret;
}

}  // namespace Renderer3D::Kernel
