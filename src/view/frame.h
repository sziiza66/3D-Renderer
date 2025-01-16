#pragma once

#include <SFML/Graphics.hpp>

#include <cstddef>

namespace Renderer3D::Kernel {

struct Color {
    sf::Uint8 r = 0;
    sf::Uint8 g = 0;
    sf::Uint8 b = 0;
};

class Frame {
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

private:
    std::vector<std::vector<Color>> data_;
};

}  // namespace Renderer3D::Kernel
