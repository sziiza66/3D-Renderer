#include "screen.h"

#include <cassert>
#include <stdexcept>

namespace Renderer3D::GUI {

Screen::Screen(Height height, Width width)
    : window_(sf::VideoMode(static_cast<unsigned int>(width), static_cast<unsigned int>(height)), "3D Renderer") {
    assert(static_cast<unsigned int>(height) != 0);
    assert(static_cast<unsigned int>(width) != 0);

    if (!texture_.create(static_cast<unsigned int>(width), static_cast<unsigned int>(height))) {
        throw std::runtime_error("Unable to create texture.\n");
    }
    sprite_.setTexture(texture_);
    window_.setKeyRepeatEnabled(false);
}

void Screen::Display(const Frame& frame) {
    // Я выяснил, что ботлнек производительности был в том, что я делал лишнее копирование, когда передавал в функцию
    // sfml массив Uint8, так вот с этой функций я могу его передавать напрямую, без копирования.
    // Но это ещё не все, я попробовал сделать data_ в классе Frame вида std::vector<sf:Uint8> и сделать отдельную
    // функцию, принимающую Color и координаты пикселя, всё, чтобы не использовать этот каст, но рендер стал рабтать
    // заметно медленнее. Это техничеки не UB, как я понял, но если есть способ это убрать и оставить
    // производительность, хотел бы использовать его, подскажи, пожалуйста.
    texture_.update(reinterpret_cast<const sf::Uint8*>(frame.GetPixels()));
    window_.clear();
    window_.draw(sprite_);
    window_.display();
}

void Screen::CloseWindow() {
    window_.close();
}

bool Screen::PollWindowEvent(sf::Event& event) {
    return window_.pollEvent(event);
}

bool Screen::IsWindowOpen() {
    return window_.isOpen();
}

}  // namespace Renderer3D::GUI
