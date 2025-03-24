#pragma once

#include "color.h"
#include "../aliases.h"

namespace Renderer3D::Kernel {

class Frame {
    // Нужно из-за специфики sfml.
    struct ColorWithAlpha {
        Color color;
        uint8_t alpha = kDefaultAlpha;

        static constexpr uint8_t kDefaultAlpha = 255;
    };

public:
    Frame() = default;
    Frame(FrameHeight height, FrameWidth width);

    Color& operator()(size_t x, size_t y);
    const Color& operator()(size_t x, size_t y) const;

    void FillWithBlackColor();
    [[nodiscard]] ssize_t GetHeight() const;
    [[nodiscard]] ssize_t GetWidth() const;
    [[nodiscard]] const ColorWithAlpha* Data() const;

private:
    static constexpr ColorWithAlpha kWhite = {0, 0, 0, ColorWithAlpha::kDefaultAlpha};

private:
    std::vector<ColorWithAlpha> data_;
    ssize_t width_ = 0;
};

}  // namespace Renderer3D::Kernel
