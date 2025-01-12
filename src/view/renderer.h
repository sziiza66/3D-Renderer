#pragma once

#include "../model/entity.h"

namespace view {

template <typename Rasterizer>
class Renderer {
public:
    Renderer(size_t width, size_t height, const entity::Camera& cam) {
        SetCam(width, height, cam);
    }

    void SetCam(size_t width, size_t height, const entity::Camera& cam) {
        assert(width != 0);
        assert(height != 0);

        rasterizer_.SetResolution(width, height);
        frustrum_ =
            Eigen::Matrix4d{{cam.GetNearDist() * 2 / (cam.GetRightIntersectX() - cam.GetLeftIntersectX()), 0,
                             (cam.GetRightIntersectX() + cam.GetLeftIntersectX()) /
                                 (cam.GetRightIntersectX() - cam.GetLeftIntersectX()),
                             0},
                            {0, cam.GetNearDist() * 2 / (cam.GetTopIntersectY() - cam.GetBottomIntersectY()),
                             (cam.GetTopIntersectY() + cam.GetBottomIntersectY()) /
                                 (cam.GetTopIntersectY() - cam.GetBottomIntersectY()),
                             0},
                            {0, 0, -(cam.GetFarDist() + cam.GetNearDist()) / (cam.GetFarDist() - cam.GetNearDist()),
                             cam.GetNearDist() * cam.GetFarDist() * 2 / (cam.GetFarDist() - cam.GetNearDist())},
                            {0, 0, -1, 0}};
    }

    void RenderFrame(sf::Uint8* frame, const Eigen::Matrix4d& camera_pos,
                     const std::vector<std::pair<entity::Property, entity::Triangle>>& triangles) {
        auto cam_inverse = camera_pos.inverse();

        rasterizer_.Clear();
        memset(frame, 0, rasterizer_.GetHeight() * rasterizer_.GetWidth() * 4);

        for (const auto pair : triangles) {
            entity::Triangle transformed_triangle = frustrum_ * cam_inverse * pair.first.GetPos() * pair.second;
            for (uint8_t j = 0; j < 3; ++j) {
                for (uint8_t i = 0; i < 3; ++i) {
                    transformed_triangle(j, i) /= transformed_triangle(3, i);
                }
            }
            rasterizer_(frame, transformed_triangle, pair.first.GetColor());
        }
    }

private:
    Rasterizer rasterizer_;
    Eigen::Matrix4d frustrum_;
};

}  // namespace view
