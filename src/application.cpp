#include "application.h"

namespace Renderer3D {

Application::Application()
    : spectator_(kScreenWidth * 1.0 / kScreenHeight),
      screen_(ScreenHeight{kScreenHeight}, ScreenWidth{kScreenWidth}),
      frame_(Frame::Height{kScreenHeight}, Frame::Width{kScreenWidth}) {
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
        UpdateFrame();
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

void Application::UpdateFrame() {
    frame_ = renderer_.RenderFrame(world_.Objects(), spectator_.Position(), spectator_.Camera(), std::move(frame_));
    screen_.Display(frame_);
}

}  // namespace Renderer3D
