#pragma once

#include "color.h"

#include <cstddef>

namespace Renderer3D::Kernel {

class Frame {
    // Нужно из-за специфики sfml.
    struct ColorWithAlpha {
        Color color;
        sf::Uint8 alpha = DefaultAlpha;
    };

public:
    Frame();

    Frame(size_t height, size_t width);

    Frame(const Frame&) = default;

    Frame(Frame&&) noexcept = default;

    Frame& operator=(const Frame&) = default;

    Frame& operator=(Frame&&) noexcept = default;

    ~Frame() = default;

    Color& operator()(size_t x, size_t y);

    const Color& operator()(size_t x, size_t y) const;

    [[nodiscard]] size_t GetHeight() const;

    [[nodiscard]] size_t GetWidth() const;

    void Clear();

    [[nodiscard]] const ColorWithAlpha* GetPixels() const;

private:
    std::vector<ColorWithAlpha> data_;
    size_t height_;
    size_t width_;

private:
    static constexpr sf::Uint8 DefaultAlpha = 255;
};

}  // namespace Renderer3D::Kernel
