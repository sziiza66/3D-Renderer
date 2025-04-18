#pragma once

#include <Eigen/Dense>

namespace Renderer3D {

using Matrix4 = Eigen::Matrix4d;
using Matrix3 = Eigen::Matrix3d;
using Vector4 = Eigen::Vector4d;
using Vector3 = Eigen::Vector3d;
// Я нашел Eigen::Transform<double, 3, Eigen::Affine>, которая предназначена именно для моих целей.
// Eigen::Matrix<double, 4, 3> так хорошо сменить не удалось, есть class Eigen::Homogeneous<MatrixType, _Direction>, но
// там возникают проблемы интеграции, которые я не смог сходу решить.
using TriMatrix = Eigen::Matrix<double, 4, 3>;
using AffineTransform = Eigen::Transform<double, 3, Eigen::Affine>;
using ConstVertexRef = Eigen::Block<const Eigen::Matrix<double, 4, 3>, 4, 1, true>;
using VertexRef = Eigen::Block<Eigen::Matrix<double, 4, 3>, 4, 1, true>;
using NormalRef = Eigen::Block<Eigen::Matrix<double, 3, 3>, 3, 1, true>;

}  // namespace Renderer3D
