#include "application.h"

namespace Renderer3D {

const Matrix4 Application::kRightTurn{
    {1, 0, 0, 0}, {0, 0.999848, -0.0174524, 0}, {0, 0.0174524, 0.999848, 0}, {0, 0, 0, 1}};
const Matrix4 Application::kLeftTurn{kRightTurn.inverse()};

Application::Application()
    : model_({}, {std::numbers::pi / 2, kNearPlaneDistance, kFarPlaneDistance, static_cast<double>(kWidth) / kHeight},
             Matrix4::Identity(), {}),
      view_({static_cast<Height>(kHeight), static_cast<Width>(kWidth)}, {kHeight, kWidth}) {

    // Магические числа, да, потом буду мир в файле хранить, а тут читать.
    Object obj;
    Triangle t1(TriMatrix{{0, 0.5, -0.5}, {0, 0.5, 0.5}, {0, 0.5, -0.5}, {1, 1, 1}}, Color{255, 0, 0});
    Triangle t2(TriMatrix{{0, 1, 0}, {0, 0, 1}, {0, 0, 0}, {1, 1, 1}}, Color{0, 255, 0});
    Triangle t3(TriMatrix{{0.4, 0.5, 0}, {1, -1, 0}, {1, 0.5, -4}, {1, 1, 1}}, Color{0, 0, 255});

    obj.PushTriangle(t1);
    obj.PushTriangle(t2);
    obj.PushTriangle(t3);
    Matrix4 pos = Matrix4::Identity();
    model_.world.PushObject(pos, obj);
}

void Application::Run() {
    // Угол поворота камеры, нужен, чтобы правильно обрабатывать движение вперед/назад/влево/вправо.
    double alpha = 0;

    while (view_.screen.IsWindowOpen()) {
        sf::Event event{};
        while (view_.screen.PollWindowEvent(event)) {
            switch (event.type) {
                case sf::Event::Closed:
                    view_.screen.CloseWindow();
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

void Application::HandleLeft(double alpha) {
    model_.camera_pos(2, 3) += kMovementCoefficient * sin(alpha);
    model_.camera_pos(1, 3) += kMovementCoefficient * cos(alpha);
}

void Application::HandleRight(double alpha) {
    model_.camera_pos(2, 3) -= kMovementCoefficient * sin(alpha);
    model_.camera_pos(1, 3) -= kMovementCoefficient * cos(alpha);
}

void Application::HandleForward(double alpha) {
    model_.camera_pos(1, 3) -= kMovementCoefficient * sin(alpha);
    model_.camera_pos(2, 3) += kMovementCoefficient * cos(alpha);
}

void Application::HandleBackward(double alpha) {
    model_.camera_pos(1, 3) += kMovementCoefficient * sin(alpha);
    model_.camera_pos(2, 3) -= kMovementCoefficient * cos(alpha);
}

void Application::HandleTurnRight(double& alpha) {
    model_.camera_pos *= kRightTurn;
    alpha += std::numbers::pi / kAngleCoefficient;
}

void Application::HandleTurnLeft(double& alpha) {
    model_.camera_pos *= kLeftTurn;
    alpha -= std::numbers::pi / kAngleCoefficient;
}

void Application::UpdateFrame() {
    view_.frame = model_.renderer.RenderFrame(model_.world.GetObjects(), model_.camera_pos, model_.camera,
                                              std::move(view_.frame));
    view_.screen.Display(view_.frame);
}

}  // namespace Renderer3D
