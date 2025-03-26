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
    using Renderer = Kernel::Renderer;
    using Spectator = Kernel::Spectator;
    using World = Kernel::World;

    struct KeyHandlerAssociation {
        sf::Keyboard::Key key;
        void (*handler)(Kernel::Spectator*);
    };

public:
    Application();

    void Run();

private:
    void DrawFrame(const Frame& frame, const sf::Sprite& sprite, sf::Texture* texture);
    void HandleLoopIteration(const sf::Sprite& sprite, sf::Texture* texture);

private:
    static void HandleUp(Spectator* spectator);
    static void HandleDown(Spectator* spectator);
    static void HandleLeft(Spectator* spectator);
    static void HandleRight(Spectator* spectator);
    static void HandleForward(Spectator* spectator);
    static void HandleBackward(Spectator* spectator);
    static void HandleTurnRight(Spectator* spectator);
    static void HandleTurnLeft(Spectator* spectator);

private:
    static constexpr size_t kDefaultWindowWidth = 1800;
    static constexpr size_t kDefaultWindowHeight = 900;
    static constexpr const char* kWindowName = "3D Renderer";
    // Моя попытка сделать обработку клавиш компактной и легко модифицируемой, если у этого варианта есть минусы, или
    // есть лучший аналог, я хотел бы знать.
    // Хотя один минус я вижу -- приходится задавать размер массива вручную, может заменить это сырой массив?
    // А ещё constexpr в этом случае вряд ли отличается от const (я не совсем уверен), но по идее это не проблема?
    static constexpr std::array<KeyHandlerAssociation, 8> UsedKeysMapping = {
        KeyHandlerAssociation{sf::Keyboard::Z, HandleUp},       KeyHandlerAssociation{sf::Keyboard::X, HandleDown},
        KeyHandlerAssociation{sf::Keyboard::A, HandleLeft},     KeyHandlerAssociation{sf::Keyboard::D, HandleRight},
        KeyHandlerAssociation{sf::Keyboard::W, HandleForward},  KeyHandlerAssociation{sf::Keyboard::S, HandleBackward},
        KeyHandlerAssociation{sf::Keyboard::Q, HandleTurnLeft}, KeyHandlerAssociation{sf::Keyboard::E, HandleTurnRight},
    };

private:
    Spectator spectator_;
    sf::RenderWindow window_;
    Renderer renderer_;
    Frame frame_;
    World world_;
};

}  // namespace Renderer3D
