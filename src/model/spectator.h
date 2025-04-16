#pragma once

#include "camera.h"

namespace Renderer3D::Kernel {

// Я решил создать что-то типа игрока или наблюдателя, чтобы камера и её позиция были связаны между собой, также здесь
// будут находиться все методы, выполняющие изменение позиции камеры. Тем более для корректного перемещения нужна не
// только позиция камеры, а так же дополнительные переменные как x_axis_polar_angle_, что придает ещё больше смысла
// этому классу.
class Spectator {
public:
    // Это максимально дефолтный конструктор данного класса, я считаю, что задавать ratio дефолтно плохая идея, т.к. в
    // типичной ситуации, оно должно соответствовать отношению сторон какого-то окна, а придав этой переменной значение
    // по умолчанию, мы типичную ситуацию делаем нетипичной.
    Spectator() = delete;
    explicit Spectator(double ratio);
    Spectator(double ratio, double movement_speed);

    [[nodiscard]] const class Camera& Camera() const;
    [[nodiscard]] const AffineTransform& Position() const;

    void TurnLeft();
    void TurnRight();
    void MoveForward();
    void MoveBackward();
    void MoveRight();
    void MoveLeft();
    void MoveUp();
    void MoveDown();

private:
    static constexpr double kDefaultMovementSpeed = 0.01;
    static constexpr double kDefaultAngularSpeed = std::numbers::pi / 180;
    static constexpr double kDefaultPolarAngle = 0;

private:
    class Camera camera_;
    AffineTransform position_;
    AffineTransform left_rotation_;
    AffineTransform right_rotation_;
    double movement_speed_;
    double angular_speed_;
    double x_axis_polar_angle_;
};

}  // namespace Renderer3D::Kernel
