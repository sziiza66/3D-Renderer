#pragma once
#include <Eigen/Dense>

#include "camera.h"
#include "color.h"
#include "position.h"

namespace Renderer3D::Kernel {

class Triangle {
    using Matrix = Eigen::Matrix<double, 4, 3>;

public:
    Triangle();

    explicit Triangle(Matrix matrix, Color color);

    double operator()(long x, long y) const;

    Eigen::Vector4d operator()(long x) const;

    [[nodiscard]] const Matrix& GetPointsMatrix() const;

    [[nodiscard]] Color GetColor() const;

    void ApplyPosition(const Position& pos);

    void ApplyFrustrum(const Camera& cam);

private:
    void NormalizeFourthCoordinate();

private:
    Matrix matrix_;
    Color color_;

private:
    const static Matrix DefaultCoordinates;
};

Triangle operator*(const Position& pos, const Triangle& triangle);

}  // namespace Renderer3D::Kernel
