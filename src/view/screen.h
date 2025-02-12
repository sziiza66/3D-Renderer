#pragma once

#include <SFML/Graphics.hpp>

#include "../model/frame.h"

namespace Renderer3D::GUI {

class Screen {
    using Frame = Renderer3D::Kernel::Frame;

public:
    enum class Height : unsigned int;
    enum class Width : unsigned int;

    Screen(Height height, Width width);

    void Display(const Frame& frame);

    void CloseWindow();

    bool PollWindowEvent(sf::Event& event);

    bool IsWindowOpen();

private:
    sf::Texture texture_;
    sf::Sprite sprite_;
    sf::RenderWindow window_;
};

}  // namespace Renderer3D::GUI
