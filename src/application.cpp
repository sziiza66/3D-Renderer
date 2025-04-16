#include "application.h"

#include <fstream>

#include "linalg.h"
#include "model/utility/simple_obj_parse.h"

namespace Renderer3D {
namespace {

sf::Texture CreateTexture(size_t width, size_t height) {
    sf::Texture ret;
    if (!ret.create(width, height)) {
        throw std::runtime_error("Unable to create texture.\n");
    }
    return ret;
}

}  // namespace

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
    star_pos_ = &world_.Objects()[0].obj.Subobjects()[0].pos;
    planet_pos_ = &(world_.Objects()[0].obj.Subobjects()[1].obj.Subobjects()[0].pos);
    moon_pos_ = &(world_.Objects()[0].obj.Subobjects()[1].obj.Subobjects()[1].pos);
    system_pos_ = &(world_.Objects()[0].obj.Subobjects()[1].pos);

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


    star_pos_->rotate(Eigen::AngleAxisd(std::numbers::pi / 1800, Vector3::UnitX()));
    planet_pos_->rotate(Eigen::AngleAxisd(std::numbers::pi / 180, Vector3::UnitX()));
    moon_pos_->rotate(Eigen::AngleAxisd(-std::numbers::pi / 180, Vector3::UnitZ()));
    system_pos_->translation() = Eigen::AngleAxisd(std::numbers::pi / 500, Vector3::UnitX()) * system_pos_->translation();
    moon_pos_->translation() = Eigen::AngleAxisd(-std::numbers::pi / 180, Vector3::UnitX()) * moon_pos_->translation();

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

    std::ifstream planet_obj("Planet.obj");
    Object planet =
        Kernel::ParseObj(planet_obj, kDefaultDiffuseColor, kDefaultSpecularColor, kDefaultSpecularPower, 3, "planet");

    std::ifstream moon_obj("Moon.obj");
    Object moon =
        Kernel::ParseObj(moon_obj, kDefaultDiffuseColor, kDefaultSpecularColor, kDefaultSpecularPower, 1, "moon");

    std::ifstream star_obj("Star.obj");
    Object star =
        Kernel::ParseObj(star_obj, kDefaultDiffuseColor, kDefaultSpecularColor, kDefaultSpecularPower, 7, "star");

    AffineTransform planet_pos(AffineTransform::Identity());
    planet_pos.translation() += Vector3{0, 0, 24};
    planet_pos.rotate(Matrix3{Eigen::AngleAxisd(std::numbers::pi / 15, Vector3::UnitX())} *
                    Matrix3{Eigen::AngleAxisd(-std::numbers::pi / 12, Vector3::UnitZ())} *
                    Matrix3{Eigen::AngleAxisd(-std::numbers::pi / 10, Vector3::UnitY())});

    AffineTransform star_pos(AffineTransform::Identity());
    star_pos.translation() += Vector3{-15, 0, 30};
    star_pos.rotate(Matrix3{Eigen::AngleAxisd(std::numbers::pi / 10, Vector3::UnitX())} *
                    Matrix3{Eigen::AngleAxisd(std::numbers::pi / 5, Vector3::UnitZ())} *
                    Matrix3{Eigen::AngleAxisd(std::numbers::pi / 6, Vector3::UnitY())});

    AffineTransform moon_pos(AffineTransform::Identity());
    moon_pos.translation() += Vector3{0, 0, 7};
    moon_pos.rotate(Matrix3{Eigen::AngleAxisd(-std::numbers::pi / 24, Vector3::UnitX())} *
                    Matrix3{Eigen::AngleAxisd(std::numbers::pi / 26, Vector3::UnitZ())} *
                    Matrix3{Eigen::AngleAxisd(-std::numbers::pi / 20, Vector3::UnitY())});

    Object planet_center;
    planet_center.PushSubObject(AffineTransform::Identity(), std::move(planet));
    planet_center.PushSubObject(moon_pos, std::move(moon));

    star.PushPointLightSource(PointLightSource{{1.0, 1.0, 0.85}, 0.000001, 0.0008, 0.002});

    Object star_center;
    star_center.PushSubObject(AffineTransform::Identity(), std::move(star));
    star_center.PushSubObject(planet_pos, std::move(planet_center));


    ret.PushObject(star_pos, std::move(star_center));

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
