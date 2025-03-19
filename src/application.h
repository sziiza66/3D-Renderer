#pragma once

#include "model/triangle.h"
#include "model/world.h"
#include "model/camera.h"
#include "view/screen.h"
#include "model/renderer.h"

// Я не уверен, точно ли Application должен быть в GUI, но у меня он обрабатывает управление, так что видимо да...
namespace Renderer3D {

class Application {
    using Triangle = Kernel::Triangle;
    using World = Kernel::World;
    using Camera = Kernel::Camera;
    using Renderer = Kernel::Renderer;
    using Object = Kernel::Object;
    using Color = Kernel::Color;
    using Height = GUI::Screen::Height;
    using Width = GUI::Screen::Width;
    using Screen = GUI::Screen;
    using Frame = Kernel::Frame;

    struct Model {
        World world;
        Camera camera;
        Matrix4 camera_pos;
        Renderer renderer;
    };

    struct View {
        Screen screen;
        Frame frame;
    };

public:
    Application();

    void Run();

private:
    void HandleLeft(double);
    void HandleRight(double);
    void HandleForward(double);
    void HandleBackward(double);
    void HandleTurnRight(double&);
    void HandleTurnLeft(double&);
    void UpdateFrame();

private:
    static constexpr size_t kWidth = 1800;
    static constexpr size_t kHeight = 900;
    static constexpr double kNearPlaneDistance = 0.1;
    static constexpr double kFarPlaneDistance = 1000;
    static constexpr double kMovementCoefficient = 0.01;
    static constexpr double kAngleCoefficient = 180;
    static const Matrix4 kRightTurn;
    static const Matrix4 kLeftTurn;

private:
    Model model_;
    View view_;
};

}  // namespace Renderer3D
