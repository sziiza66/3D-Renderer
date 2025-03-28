#include "application.h"

namespace Renderer3D {

Application::Application()
    : spectator_(kDefaultWindowWidth * 1.0 / kDefaultWindowHeight),
      window_(sf::VideoMode(kDefaultWindowWidth, kDefaultWindowHeight), kWindowName),
      frame_(Frame::UHeight{kDefaultWindowHeight}, Frame::UWidth{kDefaultWindowWidth}),
      world_(PopulateWorld()) {
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

    // Вся обработка клавиш сжимается в эти 4 строки, думаю, неплохо, и не должно влиять на производительность.
    for (const auto& association : UsedKeysMapping) {
        if (sf::Keyboard::isKeyPressed(association.key)) {
            association.handler(&spectator_);
        }
    }

    frame_ = renderer_.RenderFrame(world_.Objects(), spectator_.Position(), spectator_.Camera(), world_.AmbientLight(),
                                   std::move(frame_));
    DrawFrame(frame_, sprite, texture);
}

Application::World Application::PopulateWorld() {
    World ret;

    Object obj;
    Triangle t1(TriMatrix{{0, 0.5, -0.5}, {0, 0.5, 0.5}, {0, 0.5, -0.5}, {1, 1, 1}}, Matrix3::Identity(),
                Color{1, 0, 0});
    Triangle t2(TriMatrix{{0, 1, 0}, {0, 0, 1}, {0, 0, 0}, {1, 1, 1}}, Matrix3::Identity(), Color{0, 1, 0});
    Triangle t3(TriMatrix{{0.4, 0.5, 0}, {1, -1, 0}, {1, 0.5, -4}, {1, 1, 1}}, Matrix3::Identity(), Color{0, 0, 1});

    obj.PushTriangle(t1);
    obj.PushTriangle(t2);
    obj.PushTriangle(t3);
    ret.PushObject(AffineTransform::Identity(), std::move(obj));
    return ret;
}

void Application::HandleUp(Spectator* spectator) {
    spectator->MoveUp();
}

void Application::HandleDown(Spectator* spectator) {
    spectator->MoveDown();
}

void Application::HandleLeft(Spectator* spectator) {
    spectator->MoveLeft();
}

void Application::HandleRight(Spectator* spectator) {
    spectator->MoveRight();
}

void Application::HandleForward(Spectator* spectator) {
    spectator->MoveForward();
}

void Application::HandleBackward(Spectator* spectator) {
    spectator->MoveBackward();
}

void Application::HandleTurnRight(Spectator* spectator) {
    spectator->TurnRight();
}

void Application::HandleTurnLeft(Spectator* spectator) {
    spectator->TurnLeft();
}

}  // namespace Renderer3D
