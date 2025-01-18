#pragma once
#include <Eigen/Dense>
#include <SFML/Graphics.hpp>

namespace entity {

using Plane = Eigen::Vector4d;

class Camera {
public:
    Camera(double fov, double near_dist, double far_dist, double ratio);

    double GetRatio() const {
        return ratio_;
    }

    double GetNearDist() const {
        return near_dist_;
    }

    double GetFarDist() const {
        return far_dist_;
    }

    double GetLeftIntersectX() const {
        return left_intersect_x_;
    }

    double GetRightIntersectX() const {
        return right_intersect_x_;
    }

    double GetTopIntersectY() const {
        return top_intersect_y_;
    }

    double GetBottomIntersectY() const {
        return bottom_intersect_y_;
    }

private:
    entity::Plane near_;
    entity::Plane far_;
    entity::Plane left_;
    entity::Plane right_;
    entity::Plane bottom_;
    entity::Plane top_;
    double fov_;
    double near_dist_;
    double far_dist_;
    double ratio_;
    double left_intersect_x_;
    double right_intersect_x_;
    double top_intersect_y_;
    double bottom_intersect_y_;
};

}  // namespace entity
