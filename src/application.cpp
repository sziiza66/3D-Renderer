#include "application.h"

#include <fstream>

#include "linalg.h"
#include "model/utility/simple_obj_parse.h"

namespace Renderer3D {

sf::Texture CreateTexture(size_t width, size_t height) {
    sf::Texture ret;
    if (!ret.create(width, height)) {
        throw std::runtime_error("Unable to create texture.\n");
    }
    return ret;
}

Application::Application(char* file_name, double scale)
    : spectator_(kDefaultWindowWidth * 1.0 / kDefaultWindowHeight, kSpectatorMovementSpeed),
      window_(sf::VideoMode(kDefaultWindowWidth, kDefaultWindowHeight), kWindowName),
      frame_(Frame::SHeight{kDefaultWindowHeight}, Frame::SWidth{kDefaultWindowWidth}),
      world_(PopulateWorld(file_name, scale)),
      texture_(CreateTexture(kDefaultWindowWidth, kDefaultWindowHeight)) {
    sprite_.setTexture(texture_);
    window_.setKeyRepeatEnabled(false);
}

void Application::Run() {
    while (window_.isOpen()) {
        HandleLoopIteration(sprite_, &texture_);
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
    // Перенёс этот массив из полей класса сюда.
    static constexpr std::array<KeyHandlerAssociation, 11> UsedKeysMapping = {
        KeyHandlerAssociation{sf::Keyboard::Z, &Application::HandleUp},
        KeyHandlerAssociation{sf::Keyboard::X, &Application::HandleDown},
        KeyHandlerAssociation{sf::Keyboard::A, &Application::HandleLeft},
        KeyHandlerAssociation{sf::Keyboard::D, &Application::HandleRight},
        KeyHandlerAssociation{sf::Keyboard::W, &Application::HandleForward},
        KeyHandlerAssociation{sf::Keyboard::S, &Application::HandleBackward},
        KeyHandlerAssociation{sf::Keyboard::Q, &Application::HandleTurnLeft},
        KeyHandlerAssociation{sf::Keyboard::E, &Application::HandleTurnRight},
        KeyHandlerAssociation{sf::Keyboard::L, &Application::HandlePointLight},
        KeyHandlerAssociation{sf::Keyboard::K, &Application::HandleSpotLight},
        KeyHandlerAssociation{sf::Keyboard::T, &Application::HandleToggleSun}};

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
        (*this.*association.handler)(sf::Keyboard::isKeyPressed(association.key));
    }

    frame_ = renderer_.RenderFrame(world_.Objects(), spectator_.Position(), spectator_.Camera(), world_.AmbientLight(),
                                   world_.DirectionalLightSources(), std::move(frame_));

    DrawFrame(frame_, sprite, texture);
}

Application::World Application::PopulateWorld(char* file_name, double scale) {
    World ret;

    std::ifstream file_obj(file_name);
    Object obj = Kernel::ParseObj(file_obj, kDefaultDiffuseColor, kDefaultSpecularColor, kDefaultSpecularPower, scale);
    ret.PushObject(AffineTransform::Identity(), std::move(obj));

    return ret;
}

void Application::HandleUp(bool is_key_pressed) {
    if (is_key_pressed) {
        spectator_.MoveUp();
    }
}

void Application::HandleDown(bool is_key_pressed) {
    if (is_key_pressed) {
        spectator_.MoveDown();
    }
}

void Application::HandleLeft(bool is_key_pressed) {
    if (is_key_pressed) {
        spectator_.MoveLeft();
    }
}

void Application::HandleRight(bool is_key_pressed) {
    if (is_key_pressed) {
        spectator_.MoveRight();
    }
}

void Application::HandleForward(bool is_key_pressed) {
    if (is_key_pressed) {
        spectator_.MoveForward();
    }
}

void Application::HandleBackward(bool is_key_pressed) {
    if (is_key_pressed) {
        spectator_.MoveBackward();
    }
}

void Application::HandleTurnRight(bool is_key_pressed) {
    if (is_key_pressed) {
        spectator_.TurnRight();
    }
}

void Application::HandleTurnLeft(bool is_key_pressed) {
    if (is_key_pressed) {
        spectator_.TurnLeft();
    }
}

void Application::HandlePointLight(bool is_key_pressed) {
    if (!point_light_last_key_state_ && is_key_pressed) {
        Object lamp;
        lamp.PushPointLightSource(kDefaultPointLightSource);
        world_.PushObject(spectator_.Position(), std::move(lamp));
    }
    point_light_last_key_state_ = is_key_pressed;
}

void Application::HandleSpotLight(bool is_key_pressed) {
    if (!spot_light_last_key_state_ && is_key_pressed) {
        Object lamp;
        lamp.PushSpotLightSource(kDefaultSpotLightSource);
        world_.PushObject(spectator_.Position(), std::move(lamp));
    }
    spot_light_last_key_state_ = is_key_pressed;
}

void Application::HandleToggleSun(bool is_key_pressed) {
    if (!dir_light_last_key_state_ && is_key_pressed) {
        if (world_.DirectionalLightSources().empty()) {
            world_.PushDirectionalLightSource(kDefaultDirectionalLightSource);
        } else {
            world_.PopDirectionalLightSource();
        }
    }
    dir_light_last_key_state_ = is_key_pressed;
}

// Бесит, что Vector3 не может быть constexpr
const Application::SpotLightSource Application::kDefaultSpotLightSource = {
    {0, 1, 0.9}, Vector3::UnitZ(), 0.0001, 0.008, 0.2, 8000};
const Application::DirectionalLightSource Application::kDefaultDirectionalLightSource = {
    {0.1, 0.1, 0.1}, (-Vector3::UnitX() + Vector3::UnitY()).normalized()};

}  // namespace Renderer3D
