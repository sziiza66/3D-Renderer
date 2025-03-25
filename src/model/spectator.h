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
    Spectator(double ratio);

    // Я ставлю пометку [[nodiscard]] в местах, где среда подсказывает мне это сделать, я думал, я правильно понял, в
    // чём смысл этой пометки, но вопреки моим ожиданиям, эту функцию среда не посоветовала обозначить [[nodiscard]],
    // как я понял, это из-за того, что функция не const и что на может делать что-то, кроме как давать доступ к камере,
    // но это не так, я поставлю [[nodiscard]].
    [[nodiscard]] class Camera& Camera();
    [[nodiscard]] const class Camera& Camera() const;

    void TurnLeft();
    void TurnRight();
    void MoveForward();
    void MoveBackward();
    void MoveRight();
    void MoveLeft();
    void MoveUp();
    void MoveDown();

private:
    static constexpr double kDefaultFov = std::numbers::pi / 2;
    static constexpr double kDefaultNearPlaneDistance = 0.1;
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
