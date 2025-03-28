#pragma once

#include <cstdint>

namespace Renderer3D::Kernel {

struct Color {
    double r = 0;
    double g = 0;
    double b = 0;
};

// Добавил всё это для удобства в добавлении освещения и затенения, не знаю, насклько плохо, что тут теперь насрано?
Color operator+(const Color& lhs, const Color& rhs);
Color operator*(const Color& lhs, const Color& rhs);
Color operator*(double lhs, const Color& rhs);
Color operator/(const Color& lhs, double rhs);
Color& operator+=(Color& lhs, const Color& rhs);
Color& operator*=(Color& lhs, const Color& rhs);
Color& operator*=(const Color& lhs, double rhs);

struct DiscreteColor {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
};

DiscreteColor MakeDiscrete(const Color& col);

};  // namespace Renderer3D::Kernel
