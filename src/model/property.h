#pragma once

#include "position.h"

#include <Eigen/Dense>
#include <SFML/Graphics.hpp>

namespace Renderer3D::Kernel {

struct Color {
    sf::Uint8 r = 0;
    sf::Uint8 g = 0;
    sf::Uint8 b = 0;
};

struct Property {
    Position position;
    Color color;
};

}  // namespace Renderer3D::Kernel
