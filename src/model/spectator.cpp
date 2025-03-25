#include "spectator.h"

namespace Renderer3D::Kernel {

Spectator::Spectator(double ratio)
    : camera_(kDefaultFov, kDefaultNearPlaneDistance, ratio),
      position_(AffineTransform::Identity()),
      left_rotation_(Eigen::AngleAxisd(-kDefaultAngularSpeed, Vector3::UnitX())),
      right_rotation_(Eigen::AngleAxisd(kDefaultAngularSpeed, Vector3::UnitX())),
      movement_speed_(kDefaultMovementSpeed),
      angular_speed_(kDefaultAngularSpeed),
      x_axis_polar_angle_(kDefaultPolarAngle) {
}

Camera& Spectator::Camera() {
    return camera_;
}
const class Camera& Spectator::Camera() const {
    return camera_;
}

void Spectator::TurnLeft() {
    position_ = left_rotation_ * position_;
    x_axis_polar_angle_ -= angular_speed_;
}

void Spectator::TurnRight() {
    position_ = right_rotation_ * position_;
    x_axis_polar_angle_ += angular_speed_;
}

void Spectator::MoveForward() {
    position_(1, 3) -= movement_speed_ * std::sin(x_axis_polar_angle_);
    position_(2, 3) += movement_speed_ * std::cos(x_axis_polar_angle_);
}

void Spectator::MoveBackward() {
    position_(1, 3) += movement_speed_ * std::sin(x_axis_polar_angle_);
    position_(2, 3) -= movement_speed_ * std::cos(x_axis_polar_angle_);
}

void Spectator::MoveRight() {
    position_(1, 3) -= movement_speed_ * std::cos(x_axis_polar_angle_);
    position_(2, 3) -= movement_speed_ * std::sin(x_axis_polar_angle_);
}

void Spectator::MoveLeft() {
    position_(1, 3) += movement_speed_ * std::cos(x_axis_polar_angle_);
    position_(2, 3) += movement_speed_ * std::sin(x_axis_polar_angle_);
}

void Spectator::MoveUp() {
    position_(0, 3) += movement_speed_;
}

void Spectator::MoveDown() {
    position_(0, 3) -= movement_speed_;
}

}  // namespace Renderer3D::Kernel
