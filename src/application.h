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

public:
    Application();

    void Run();

private:
    void HandleUp();
    void HandleDown();
    void HandleLeft();
    void HandleRight();
    void HandleForward();
    void HandleBackward();
    void HandleTurnRight();
    void HandleTurnLeft();

    void DrawFrame(const Frame& frame, const sf::Sprite& sprite, sf::Texture* texture);
    void HandleLoopIteration(const sf::Sprite& sprite, sf::Texture* texture);

private:
    static constexpr size_t kDefaultWindowWidth = 1800;
    static constexpr size_t kDefaultWindowHeight = 900;
    static constexpr const char* kWindowName = "3D Renderer";

private:
    Spectator spectator_;
    sf::RenderWindow window_;
    Renderer renderer_;
    Frame frame_;
    World world_;
};

}  // namespace Renderer3D
