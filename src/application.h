#pragma once

#include "model/triangle.h"
#include "model/world.h"
#include "model/camera.h"
#include "view/screen.h"
#include "model/renderer.h"

namespace Renderer3D {

class Application {
    using Triangle = Kernel::Triangle;
    using World = Kernel::World;
    using Camera = Kernel::Camera;
    using Renderer = Kernel::Renderer;
    using Object = Kernel::Object;
    using Color = Kernel::Color;
    using Screen = GUI::Screen;
    using Frame = Kernel::Frame;

public:
    Application();

    void Run();

private:
    static Camera DefaultCamera();

    void HandleLeft(double);
    void HandleRight(double);
    void HandleForward(double);
    void HandleBackward(double);
    void HandleTurnRight(double&);
    void HandleTurnLeft(double&);
    void UpdateFrame();

private:
    static constexpr size_t kScreenWidth = 1800;
    static constexpr size_t kScreenHeight = 900;
    static constexpr double kNearPlaneDistance = 0.1;
    static constexpr double kFarPlaneDistance = 1000;
    static constexpr double kMovementSpeedCoefficient = 0.01;
    static constexpr double kAngleCoefficient = 180;
    static const HomoTransform kRightTurn;
    static const HomoTransform kLeftTurn;

private:
    World world_;
    Camera camera_;
    HomoTransform camera_pos_;
    Renderer renderer_;
    Screen screen_;
    Frame frame_;
};

}  // namespace Renderer3D
