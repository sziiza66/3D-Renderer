#pragma once
#include <SFML/Graphics.hpp>

namespace Renderer3D {

// Не знаю, правильно ли сделал, но мне кажется, эта структура должна быть в отдельном файле, потому что нужна она
// везде, но ни к чему конкретно не принадлежит.
struct Color {
    sf::Uint8 r = 0;
    sf::Uint8 g = 0;
    sf::Uint8 b = 0;
};

};  // namespace Renderer3D
