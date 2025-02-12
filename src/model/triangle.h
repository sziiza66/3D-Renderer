#pragma once
#include <Eigen/Dense>

#include "color.h"

namespace Renderer3D::Kernel {

// Это ещё сегодня было классом, но теперь структура, но всё еще лежит в отдельном .h файле, можно положить это в
// object.h, но надо ли? Потом object.h придётся инклюдить в рендер и растерайзер, а им не нужен Object, нужен только
// Triangle. Печально, что c++ не поддерживает include classname from file.h.
struct Triangle {
    Eigen::Matrix<double, 4, 3> matrix;
    Color color;
};

}  // namespace Renderer3D::Kernel
