#include "application.h"

namespace Renderer3D {

Application::Application()
    : camera_(DefaultCamera()),
      camera_pos_(HomoTransform::Identity()),
      screen_(ScreenHeight{kScreenHeight}, ScreenWidth{kScreenWidth}),
      frame_(FrameHeight{kScreenHeight}, FrameWidth{kScreenWidth}) {
    // Магические числа, да, потом буду мир в файле хранить, а тут читать.
    Object obj;
    Triangle t1(TriMatrix{{0, 0.5, -0.5}, {0, 0.5, 0.5}, {0, 0.5, -0.5}, {1, 1, 1}}, Color{255, 0, 0});
    Triangle t2(TriMatrix{{0, 1, 0}, {0, 0, 1}, {0, 0, 0}, {1, 1, 1}}, Color{0, 255, 0});
    Triangle t3(TriMatrix{{0.4, 0.5, 0}, {1, -1, 0}, {1, 0.5, -4}, {1, 1, 1}}, Color{0, 0, 255});

    obj.PushTriangle(t1);
    obj.PushTriangle(t2);
    obj.PushTriangle(t3);
    HomoTransform pos = HomoTransform::Identity();
    world_.PushObject(pos, std::move(obj));
}

void Application::Run() {
    // Угол поворота камеры, нужен, чтобы правильно обрабатывать движение вперед/назад/влево/вправо.
    double alpha = 0;

    while (screen_.IsWindowOpen()) {
        sf::Event event{};
        while (screen_.PollWindowEvent(event)) {
            switch (event.type) {
                case sf::Event::Closed:
                    screen_.CloseWindow();
                    break;
                default:
                    break;
            }
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            HandleLeft(alpha);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
            HandleRight(alpha);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
            HandleForward(alpha);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
            HandleBackward(alpha);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::E)) {
            HandleTurnRight(alpha);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) {
            HandleTurnLeft(alpha);
        }
        UpdateFrame();
    }
}

Application::Camera Application::DefaultCamera() {
    return {std::numbers::pi / 2, kNearPlaneDistance, kFarPlaneDistance, 1.0 * kScreenWidth / kScreenHeight};
}

void Application::HandleLeft(double alpha) {
    camera_pos_(2, 3) += kMovementSpeedCoefficient * sin(alpha);
    camera_pos_(1, 3) += kMovementSpeedCoefficient * cos(alpha);
}

void Application::HandleRight(double alpha) {
    camera_pos_(2, 3) -= kMovementSpeedCoefficient * sin(alpha);
    camera_pos_(1, 3) -= kMovementSpeedCoefficient * cos(alpha);
}

void Application::HandleForward(double alpha) {
    camera_pos_(1, 3) -= kMovementSpeedCoefficient * sin(alpha);
    camera_pos_(2, 3) += kMovementSpeedCoefficient * cos(alpha);
}

void Application::HandleBackward(double alpha) {
    camera_pos_(1, 3) += kMovementSpeedCoefficient * sin(alpha);
    camera_pos_(2, 3) -= kMovementSpeedCoefficient * cos(alpha);
}

void Application::HandleTurnRight(double& alpha) {
    camera_pos_ = camera_pos_ * kRightTurn;
    alpha += std::numbers::pi / kAngleCoefficient;
}

void Application::HandleTurnLeft(double& alpha) {
    camera_pos_ = camera_pos_ * kLeftTurn;
    alpha -= std::numbers::pi / kAngleCoefficient;
}

void Application::UpdateFrame() {
    frame_ = renderer_.RenderFrame(world_.GetObjects(), camera_pos_, camera_, std::move(frame_));
    screen_.Display(frame_);
}

const HomoTransform Application::kRightTurn{Eigen::AngleAxisd(std::numbers::pi / kAngleCoefficient, Vector3::UnitX())};
const HomoTransform Application::kLeftTurn{kRightTurn.inverse()};

}  // namespace Renderer3D
