#pragma once

#include "model/triangle.h"
#include "model/world.h"
#include "model/spectator.h"
#include "view/screen.h"
#include "model/renderer.h"

namespace Renderer3D {

class Application {
    using Camera = Kernel::Camera;
    using Color = Kernel::Color;
    using Frame = Kernel::Frame;
    using Triangle = Kernel::Triangle;
    using Object = Kernel::Object;
    using Renderer = Kernel::Renderer;
    using Screen = GUI::Screen;
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
    void UpdateFrame();

private:
    static constexpr size_t kScreenWidth = 1800;
    static constexpr size_t kScreenHeight = 900;

private:
    World world_;
    Spectator spectator_;
    Renderer renderer_;
    Screen screen_;
    Frame frame_;
};

}  // namespace Renderer3D
