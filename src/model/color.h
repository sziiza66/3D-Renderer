#pragma once

#include <cstdint>

namespace Renderer3D::Kernel {

// Не знаю, правильно ли сделал, но мне кажется, эта структура должна быть в отдельном файле, потому что нужна она
// везде, но ни к чему конкретно не принадлежит.
struct Color {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
};

};  // namespace Renderer3D::Kernel
