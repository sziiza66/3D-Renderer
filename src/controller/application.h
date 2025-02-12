#pragma once

#include "../model/world.h"
#include "../model/camera.h"
#include "../view/screen.h"
#include "../model/renderer.h"

// Я не уверен, точно ли Application должен быть в GUI, но у меня он обрабатывает управление, так что видимо да...
namespace Renderer3D::GUI {

class Application {
    struct Model {
        using World = Renderer3D::Kernel::World;
        using Camera = Renderer3D::Kernel::Camera;
        using Position = Eigen::Matrix4d;
        using Renderer = Renderer3D::Kernel::Renderer;
        World world;
        Camera camera;
        Position camera_pos;
        Renderer renderer;
    };

    struct View {
        using Screen = Renderer3D::GUI::Screen;
        using Frame = Renderer3D::Kernel::Frame;
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
    Model model_;
    View view_;

private:
    static constexpr size_t Width = 1800;
    static constexpr size_t Height = 900;
    static constexpr double NearPlaneDistance = 0.1;
    static constexpr double FarPlaneDistance = 1000;
    static constexpr double MovementCoefficient = 0.01;
    static constexpr double AngleCoefficient = 180;
    static const Eigen::Matrix4d RightTurn;
    static const Eigen::Matrix4d LeftTurn;
};

}  // namespace Renderer3D::GUI
