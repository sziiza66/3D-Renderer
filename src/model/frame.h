#pragma once

#include "color.h"

namespace Renderer3D::Kernel {

class Frame {
    // Нужно из-за специфики sfml.
    struct ColorWithAlpha {
        Color color;
        sf::Uint8 alpha = kDefaultAlpha;
    };

public:
    Frame();

    Frame(ssize_t height, ssize_t width);

    Color& operator()(ssize_t x, ssize_t y);

    const Color& operator()(ssize_t x, ssize_t y) const;

    [[nodiscard]] ssize_t GetHeight() const;

    [[nodiscard]] ssize_t GetWidth() const;

    void Clear();

    [[nodiscard]] const ColorWithAlpha* GetPixels() const;

    [[nodiscard]] ssize_t CalcYDiscreteFromRealSegment(double y, double segment_length) const;

    [[nodiscard]] ssize_t CalcXDiscreteFromRealSegment(double x, double segment_length) const;

private:
    static constexpr sf::Uint8 kDefaultAlpha = 255;

private:
    std::vector<ColorWithAlpha> data_;
    ssize_t height_;
    ssize_t width_;
};

}  // namespace Renderer3D::Kernel
