#include "application.h"

namespace Renderer3D::GUI {

const Eigen::Matrix4d Application::RightTurn{
    {1, 0, 0, 0}, {0, 0.999848, -0.0174524, 0}, {0, 0.0174524, 0.999848, 0}, {0, 0, 0, 1}};
const Eigen::Matrix4d Application::LeftTurn{RightTurn.inverse()};

Application::Application()
    : model_({}, {std::numbers::pi / 2, NearPlaneDistance, FarPlaneDistance, static_cast<double>(Width) / Height},
             Model::Position::Identity(), {}),
      view_({static_cast<Screen::Height>(Height), static_cast<Screen::Width>(Width)}, {Height, Width}) {

    // Магические числа, да, потом буду мир в файле хранить, а тут читать.
    Renderer3D::Kernel::Object obj;
    Renderer3D::Kernel::Triangle t1(
        Eigen::Matrix<double, 4, 3>{{0, 0.5, -0.5}, {0, 0.5, 0.5}, {0, 0.5, -0.5}, {1, 1, 1}},
        Renderer3D::Kernel::Color{255, 0, 0});
    Renderer3D::Kernel::Triangle t2(Eigen::Matrix<double, 4, 3>{{0, 1, 0}, {0, 0, 1}, {0, 0, 0}, {1, 1, 1}},
                                    Renderer3D::Kernel::Color{0, 255, 0});
    Renderer3D::Kernel::Triangle t3(Eigen::Matrix<double, 4, 3>{{0.4, 0.5, 0}, {1, -1, 0}, {1, 0.5, -4}, {1, 1, 1}},
                                    Renderer3D::Kernel::Color{0, 0, 255});

    obj.PushTriangle(t1);
    obj.PushTriangle(t2);
    obj.PushTriangle(t3);
    Model::Position pos = Model::Position::Identity();
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
    model_.camera_pos(2, 3) += MovementCoefficient * sin(alpha);
    model_.camera_pos(1, 3) += MovementCoefficient * cos(alpha);
}

void Application::HandleRight(double alpha) {
    model_.camera_pos(2, 3) -= MovementCoefficient * sin(alpha);
    model_.camera_pos(1, 3) -= MovementCoefficient * cos(alpha);
}

void Application::HandleForward(double alpha) {
    model_.camera_pos(1, 3) -= MovementCoefficient * sin(alpha);
    model_.camera_pos(2, 3) += MovementCoefficient * cos(alpha);
}

void Application::HandleBackward(double alpha) {
    model_.camera_pos(1, 3) += MovementCoefficient * sin(alpha);
    model_.camera_pos(2, 3) -= MovementCoefficient * cos(alpha);
}

void Application::HandleTurnRight(double& alpha) {
    model_.camera_pos *= RightTurn;
    alpha += std::numbers::pi / AngleCoefficient;
}

void Application::HandleTurnLeft(double& alpha) {
    model_.camera_pos *= LeftTurn;
    alpha -= std::numbers::pi / AngleCoefficient;
}

void Application::UpdateFrame() {
    view_.frame = model_.renderer.RenderFrame(model_.world.GetObjects(), model_.camera_pos, model_.camera,
                                              std::move(view_.frame));
    view_.screen.Display(view_.frame);
}

}  // namespace Renderer3D::GUI
