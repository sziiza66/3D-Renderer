#pragma once
#include <Eigen/Dense>
#include <cassert>

#include "entity.h"
#include "triangle.h"
#include "property.h"

namespace Renderer3D::Kernel {

class World {
public:
    World() : camera_pos_({{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}), camera_(nullptr) {
    }

    ~World() {
        delete camera_;
    }

    void CreateCamera(double fov, double near_dist, double far_dist, double ratio) {
        delete camera_;
        camera_ = new entity::Camera(fov, near_dist, far_dist, ratio);
    }

    void PushTriangle(Property&& prop, Triangle&& triangle) {
        triangles_.emplace_back(std::move(prop), std::move(triangle));
    }

    void PushTriangle(const Property& prop, const Triangle& triangle) {
        triangles_.emplace_back(prop, triangle);
    }

    const entity::Camera& GetCamera() const {
        return *camera_;
    }

    Eigen::Matrix4d& GetCameraPos() {
        return camera_pos_;
    }

    const Eigen::Matrix4d& GetCameraPos() const {
        return camera_pos_;
    }

    const std::vector<std::pair<Property, Triangle>>& GetTriangles() const {
        return triangles_;
    }

private:
    std::vector<std::pair<Property, Triangle>> triangles_;
    Eigen::Matrix4d camera_pos_;
    entity::Camera* camera_;
};

}  // namespace model
