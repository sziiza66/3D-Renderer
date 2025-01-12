#pragma once
#include <Eigen/Dense>
#include <cassert>
#include <utility>
#include <vector>

#include "entity.h"

namespace model {

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

    void PushTriangle(entity::Property&& prop, entity::Triangle&& triangle) {
        triangles_.emplace_back(std::move(prop), std::move(triangle));
    }

    void PushTriangle(const entity::Property& prop, const entity::Triangle& triangle) {
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

    const std::vector<std::pair<entity::Property, entity::Triangle>>& GetTriangles() const {
        return triangles_;
    }

private:
    std::vector<std::pair<entity::Property, entity::Triangle>> triangles_;
    Eigen::Matrix4d camera_pos_;
    entity::Camera* camera_;
};

}  // namespace model
