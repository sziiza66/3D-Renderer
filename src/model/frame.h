#pragma once

#include <cstddef>
#include <sys/types.h>
#include <vector>

#include "color.h"

namespace Renderer3D::Kernel {

class Frame {
    // Нужно из-за специфики sfml.
    struct ColorWithAlpha {
        Color color;
        uint8_t alpha = kDefaultAlpha;

        static constexpr uint8_t kDefaultAlpha = 255;
    };

public:
    enum Width : ssize_t;
    enum Height : ssize_t;

    Frame() = default;
    Frame(Height height, Width width);

    Color& operator()(size_t x, size_t y);
    const Color& operator()(size_t x, size_t y) const;

    void FillWithBlackColor();
    [[nodiscard]] size_t GetHeight() const;
    [[nodiscard]] size_t GetWidth() const;
    [[nodiscard]] const ColorWithAlpha* Data() const;

private:
    static constexpr ColorWithAlpha kWhite = {0, 0, 0, ColorWithAlpha::kDefaultAlpha};

private:
    std::vector<ColorWithAlpha> data_;
    ssize_t width_ = 0;
};

}  // namespace Renderer3D::Kernel
