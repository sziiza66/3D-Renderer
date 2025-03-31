#pragma once

#include <SFML/Graphics.hpp>

#include "model/triangle.h"
#include "model/world.h"
#include "model/spectator.h"
#include "model/renderer.h"

namespace Renderer3D {

class Application {
    using Camera = Kernel::Camera;
    using Color = Kernel::Color;
    using Frame = Kernel::Frame;
    using Triangle = Kernel::Triangle;
    using Object = Kernel::Object;
    using PointLightSource = Kernel::PointLightSource;
    using Renderer = Kernel::Renderer;
    using Spectator = Kernel::Spectator;
    using World = Kernel::World;

    struct KeyHandlerAssociation {
        sf::Keyboard::Key key;
        void (Application::*handler)();
    };

public:
    Application(char* file_name, double scale);

    void Run();

private:
    void DrawFrame(const Frame& frame, const sf::Sprite& sprite, sf::Texture* texture);
    void HandleLoopIteration(const sf::Sprite& sprite, sf::Texture* texture);

private:
    static World PopulateWorld(char* file_name, double scale);

    void HandleUp();
    void HandleDown();
    void HandleLeft();
    void HandleRight();
    void HandleForward();
    void HandleBackward();
    void HandleTurnRight();
    void HandleTurnLeft();
    void HandleLight();

private:
    static constexpr size_t kDefaultWindowWidth = 1920;
    static constexpr size_t kDefaultWindowHeight = 1080;
    static constexpr double kSpectatorMovementSpeed = 0.2;
    static constexpr const char* kWindowName = "3D Renderer";
    static constexpr PointLightSource kDefaultLightSource = {{1, 0.8, 1}, 0.0001, 0.008, 0.2};
    // Моя попытка сделать обработку клавиш компактной и легко модифицируемой, если у этого варианта есть минусы, или
    // есть лучший аналог, я хотел бы знать.
    // Хотя один минус я вижу -- приходится задавать размер массива вручную, может заменить это сырой массив?
    // А ещё constexpr в этом случае вряд ли отличается от const (я не совсем уверен), но по идее это не проблема?
    static constexpr std::array<KeyHandlerAssociation, 9> UsedKeysMapping = {
        KeyHandlerAssociation{sf::Keyboard::Z, &Application::HandleUp},
        KeyHandlerAssociation{sf::Keyboard::X, &Application::HandleDown},
        KeyHandlerAssociation{sf::Keyboard::A, &Application::HandleLeft},
        KeyHandlerAssociation{sf::Keyboard::D, &Application::HandleRight},
        KeyHandlerAssociation{sf::Keyboard::W, &Application::HandleForward},
        KeyHandlerAssociation{sf::Keyboard::S, &Application::HandleBackward},
        KeyHandlerAssociation{sf::Keyboard::Q, &Application::HandleTurnLeft},
        KeyHandlerAssociation{sf::Keyboard::E, &Application::HandleTurnRight},
        KeyHandlerAssociation{sf::Keyboard::L, &Application::HandleLight}};

private:
    Spectator spectator_;
    sf::RenderWindow window_;
    Renderer renderer_;
    Frame frame_;
    World world_;
};

}  // namespace Renderer3D
