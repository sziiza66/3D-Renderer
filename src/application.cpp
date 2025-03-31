#include "application.h"

#include <fstream>

#include "model/utility/object_util.h"
#include "model/utility/simple_obj_parse.h"

namespace Renderer3D {

Application::Application()
    : spectator_(kDefaultWindowWidth * 1.0 / kDefaultWindowHeight, kSpectatorMovementSpeed),
      window_(sf::VideoMode(kDefaultWindowWidth, kDefaultWindowHeight), kWindowName),
      frame_(Frame::SHeight{kDefaultWindowHeight}, Frame::SWidth{kDefaultWindowWidth}),
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

    // PointLightSource light({10, 8, 10}, 0, -3, 0.1);
    // Object lamp;
    // lamp.PushPointLightSource(light);
    // AffineTransform lamp_pos = AffineTransform::Identity();
    // lamp_pos.translation() += Vector3{20, 0, 20};
    // AffineTransform pos = AffineTransform::Identity();
    // pos.translation() += Vector3{0, 0, 20};
    // ret.PushObject(pos, std::move(Kernel::CreateOctahedron(10, {0.5, 0.5, 1})));
    // ret.PushObject(lamp_pos, std::move(lamp));
    // lamp_pos.translation() -= Vector3{40, 0, 0};
    // Object lamp2;
    // PointLightSource light2({2, 4, 2}, 0, -3, 0.1);
    // lamp2.PushPointLightSource(light2);
    // ret.PushObject(lamp_pos, std::move(lamp2));


    PointLightSource light({1, 0.8, 1}, 0.0001, 0.005, 0.01);
    Object lamp;
    lamp.PushPointLightSource(light);
    AffineTransform lamp_pos = AffineTransform::Identity();
    lamp_pos.translation() += Vector3{20, 3, 2};

    std::ifstream file_obj("teapot.obj");
    Object obj = Kernel::ParseObj(file_obj, 0, {1, 1, 1}, {0.9, 0.8, 0.5}, 50, 1);
    AffineTransform obj_pos = AffineTransform{Eigen::AngleAxisd(-std::numbers::pi / 2, Vector3::UnitY())};
    AffineTransform rot = AffineTransform{Eigen::AngleAxisd(-std::numbers::pi / 5, Vector3::UnitZ())};
    obj_pos = rot * obj_pos;
    obj_pos.translation() += Vector3{0, 2, 0};

    ret.PushObject(lamp_pos, std::move(lamp));
    ret.PushObject(obj_pos, std::move(obj));



    // PointLightSource light({1, 0.8, 1}, 0.0001, 0.002, 0.001);
    // Object lamp;
    // lamp.PushPointLightSource(light);
    // AffineTransform lamp_pos = AffineTransform::Identity();
    // double scale = 4; 
    // lamp_pos.translation() += Vector3{20, 13, 20};

    // Triangle tr1;
    // tr1.vertices = TriMatrix{{0 * scale, 2.4323978 * scale, 4.99312 * scale}, {0 * scale, 4.123 * scale, 2.11123 * scale}, {0 * scale, 1 * scale, -0.5423 * scale}, {1, 1, 1}};
    // tr1.diffuse_reflection_color = {1, 1, 1};
    // Triangle tr2;
    // tr2.vertices = TriMatrix{{10 * scale, 2.4323978 * scale, 4.99312 * scale}, {30 * scale, 4.123 * scale, 2.11123 * scale}, {10 * scale, 1 * scale, -0.5423 * scale}, {1, 1, 1}};
    // tr2.diffuse_reflection_color = {1, 1, 1};
    // Object obj;
    // tr1.diffuse_reflection_color = {1, 0, 0};
    // obj.PushTriangle(tr1);
    // obj.PushTriangle(tr2);

    // AffineTransform obj_pos = AffineTransform{Eigen::AngleAxisd(-std::numbers::pi / 2, Vector3::UnitY())};
    // AffineTransform rot = AffineTransform{Eigen::AngleAxisd(-std::numbers::pi / 8, Vector3::UnitZ())};
    // obj_pos = rot * obj_pos;


    // ret.PushObject(lamp_pos, std::move(lamp));ret.PushObject(lamp_pos, std::move(lamp));
    // ret.PushObject(obj_pos, std::move(obj));

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
