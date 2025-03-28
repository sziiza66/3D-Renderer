#include "color.h"

#include <limits>

namespace Renderer3D::Kernel {

Color operator+(const Color& lhs, const Color& rhs) {
    return {lhs.r + rhs.r, lhs.g + rhs.g, lhs.b + rhs.b};
}

Color operator*(const Color& lhs, const Color& rhs) {
    return {lhs.r * rhs.r, lhs.g * rhs.g, lhs.b * rhs.b};
}

Color operator*(double lhs, const Color& rhs) {
    return {lhs * rhs.r, lhs * rhs.g, lhs * rhs.b};
}

Color operator/(const Color& lhs, double rhs) {
    return {lhs.r / rhs, lhs.g / rhs, lhs.b / rhs};
}

Color& operator+=(Color& lhs, const Color& rhs) {
    lhs.r += rhs.r;
    lhs.g += rhs.g;
    lhs.b += rhs.b;
    return lhs;
}

Color& operator*=(Color& lhs, const Color& rhs) {
    lhs.r *= rhs.r;
    lhs.g *= rhs.g;
    lhs.b *= rhs.b;
    return lhs;
}

Color& operator*=(Color& lhs, double rhs) {
    lhs.r *= rhs;
    lhs.g *= rhs;
    lhs.b *= rhs;
    return lhs;
}

DiscreteColor MakeDiscrete(const Color& col) {
    return DiscreteColor(col.r * std::numeric_limits<uint8_t>::max(), col.g * std::numeric_limits<uint8_t>::max(),
                         col.b * std::numeric_limits<uint8_t>::max());
}

}  // namespace Renderer3D::Kernel
