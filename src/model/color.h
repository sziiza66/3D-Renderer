#pragma once

#include <cstdint>

namespace Renderer3D::Kernel {

struct DiscreteColor {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
};

struct Color {
    double r = 0;
    double g = 0;
    double b = 0;
};

DiscreteColor MakeDiscrete(const Color& col);

};  // namespace Renderer3D::Kernel
