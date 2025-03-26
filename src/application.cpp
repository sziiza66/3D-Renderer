#include "application.h"

namespace Renderer3D {

Application::Application()
    : spectator_(kDefaultWindowWidth * 1.0 / kDefaultWindowHeight),
      frame_(Frame::Height{kDefaultWindowHeight}, Frame::Width{kDefaultWindowWidth}),
      window_(sf::VideoMode(kDefaultWindowWidth, kDefaultWindowHeight), kWindowName) {
    // Магические числа, да, потом буду мир в файле хранить, а тут читать.
    Object obj;
    Triangle t1(TriMatrix{{0, 0.5, -0.5}, {0, 0.5, 0.5}, {0, 0.5, -0.5}, {1, 1, 1}}, Color{255, 0, 0});
    Triangle t2(TriMatrix{{0, 1, 0}, {0, 0, 1}, {0, 0, 0}, {1, 1, 1}}, Color{0, 255, 0});
    Triangle t3(TriMatrix{{0.4, 0.5, 0}, {1, -1, 0}, {1, 0.5, -4}, {1, 1, 1}}, Color{0, 0, 255});

    obj.PushTriangle(t1);
    obj.PushTriangle(t2);
    obj.PushTriangle(t3);
    AffineTransform pos = AffineTransform::Identity();
    world_.PushObject(pos, std::move(obj));
}

void Application::Run() {
    sf::Texture texture;
    sf::Sprite sprite;
    if (!texture.create(kDefaultWindowWidth, kDefaultWindowHeight)) {
        throw std::runtime_error("Unable to create texture.\n");
    }
    sprite.setTexture(texture);
    window_.setKeyRepeatEnabled(false);

    while (window_.isOpen()) {
        HandleLoopIteration(sprite, &texture);
    }
}

void Application::HandleUp() {
    spectator_.MoveUp();
}

void Application::HandleDown() {
    spectator_.MoveDown();
}

void Application::HandleLeft() {
    spectator_.MoveLeft();
}

void Application::HandleRight() {
    spectator_.MoveRight();
}

void Application::HandleForward() {
    spectator_.MoveForward();
}

void Application::HandleBackward() {
    spectator_.MoveBackward();
}

void Application::HandleTurnRight() {
    spectator_.TurnRight();
}

void Application::HandleTurnLeft() {
    spectator_.TurnLeft();
}

void Application::DrawFrame(const Frame& frame, const sf::Sprite& sprite, sf::Texture* texture) {
    assert(sprite.getTexture() == texture);
    // Я выяснил, что ботлнек производительности был в том, что я делал лишнее копирование, когда передавал в функцию
    // sfml массив Uint8, так вот с этой функций я могу его передавать напрямую, без копирования.
    // Но это ещё не все, я попробовал сделать data_ в классе Frame вида std::vector<sf:Uint8> и сделать отдельную
    // функцию, принимающую Color и координаты пикселя, всё, чтобы не использовать этот каст, но рендер стал рабтать
    // заметно медленнее. Это техничеки не UB, как я понял, но если есть способ это убрать и оставить
    // производительность, хотел бы использовать его.
    texture->update(reinterpret_cast<const sf::Uint8*>(frame.Data()));
    window_.clear();
    window_.draw(sprite);
    window_.display();
}

void Application::HandleLoopIteration(const sf::Sprite& sprite, sf::Texture* texture) {
    assert(sprite.getTexture() == texture);

    sf::Event event{};
    while (window_.pollEvent(event)) {
        switch (event.type) {
            case sf::Event::Closed:
                window_.close();
                return;
            default:
                return;
        }
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z)) {
        HandleUp();
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::X)) {
        HandleDown();
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
        HandleLeft();
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
        HandleRight();
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
        HandleForward();
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
        HandleBackward();
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::E)) {
        HandleTurnRight();
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) {
        HandleTurnLeft();
    }

    frame_ = renderer_.RenderFrame(world_.Objects(), spectator_.Position(), spectator_.Camera(), std::move(frame_));
    DrawFrame(frame_, sprite, texture);
}

}  // namespace Renderer3D
