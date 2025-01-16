#pragma once
#include <Eigen/Dense>

namespace Renderer3D::Kernel {

class Triangle {
public:
    Triangle();

    explicit Triangle(const Eigen::Matrix<double, 4, 3>& matrix);

    double operator()(long x, long y) const;

    [[nodiscard]] const Eigen::Matrix<double, 4, 3>& GetPointsMatrix() const;

private: 
    void NormalizeFourthCoordinate();

private:
    Eigen::Matrix<double, 4, 3> matrix_;

private:
    const static Eigen::Matrix<double, 4, 3> kDEFAULTTRIANGLE;
};

}  // namespace Renderer3D::Kernel
