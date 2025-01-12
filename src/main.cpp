
#include "model/entity.h"
#include "model/world.h"
#include "view/renderer.h"
#include "view/rasterizer.h"

#include <iostream>

int main() {

    entity::Triangle my_first_triangle{{0, 0.5, -0.5}, {0, 0.5, 0.5}, {0, 0.5, -0.5}, {1, 1, 1}};
    Eigen::Matrix4d pos1{{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 2}, {0, 0, 0, 1}};

    entity::Triangle my_second_triangle{{0, 1, 0}, {0, 0, 1}, {0, 0, 0}, {1, 1, 1}};
    Eigen::Matrix4d pos2{{1, 0, 0, 0}, {0, 1, 0, 2}, {0, 0, 1, 0}, {0, 0, 0, 1}};

    entity::Triangle my_third_triangle{{0.4, 0.5, 0}, {1, -1, 0}, {1, 0.5, -4}, {1, 1, 1}};
    Eigen::Matrix4d pos3{{1, 0, 0, 0}, {0, 1, 0, -1}, {0, 0, 1, -1}, {0, 0, 0, 1}};

    model::World world;
    world.PushTriangle(entity::Property(pos1, 255, 0, 0), my_first_triangle);
    world.PushTriangle(entity::Property(pos2, 0, 255, 0), my_second_triangle);
    world.PushTriangle(entity::Property(pos3, 0, 0, 255), my_third_triangle);

    constexpr size_t kWidth = 1800;
    constexpr size_t kHeight = 900;
    world.CreateCamera(std::numbers::pi / 2, 0.1, 1000, static_cast<double>(kWidth) / kHeight);

    view::Renderer<view::BufferRasterizer> renderer(kWidth, kHeight, world.GetCamera());

    sf::Texture texture;
    if (!texture.create(kWidth, kHeight)) {
        std::cout << "Unable to create texture.\n";
        return -1;
    }

    sf::Uint8* pixels = new sf::Uint8[kWidth * kHeight * 4];
    if (!pixels) {
        std::cout << "Unable to allocate frame.\n";
        return -1;
    }
    texture.update(pixels);

    sf::Sprite sprite;
    sprite.setTexture(texture);

    sf::RenderWindow window(sf::VideoMode(kWidth, kHeight), "3D Renderer");
    window.setKeyRepeatEnabled(false);

    renderer.RenderFrame(pixels, world.GetCameraPos(), world.GetTriangles());
    texture.update(pixels);
    window.clear();
    sleep(1);
    window.draw(sprite);
    window.display();

    Eigen::Matrix4d rturn{{1, 0, 0, 0}, {0, 0.999848, -0.0174524, 0}, {0, 0.0174524, 0.999848, 0}, {0, 0, 0, 1}};
    Eigen::Matrix4d lturn = rturn.inverse();
    double alpha = 0;

    while (window.isOpen()) {
        bool is_frame_update_needed = false;
        bool left = false;
        bool right = false;
        bool forward = false;
        bool backward = false;
        bool turn_clockwise = false;
        bool turn_counterclockwise = false;
        sf::Event event;
        while (window.pollEvent(event)) {
            switch (event.type) {
                case sf::Event::Closed:
                    window.close();
                    break;
                default:
                    break;
            }
        }
        left = sf::Keyboard::isKeyPressed(sf::Keyboard::A);
        right = sf::Keyboard::isKeyPressed(sf::Keyboard::D);
        forward = sf::Keyboard::isKeyPressed(sf::Keyboard::W);
        backward = sf::Keyboard::isKeyPressed(sf::Keyboard::S);
        turn_clockwise = sf::Keyboard::isKeyPressed(sf::Keyboard::E);
        turn_counterclockwise = sf::Keyboard::isKeyPressed(sf::Keyboard::Q);
        if (left) {
            is_frame_update_needed = true;
            world.GetCameraPos()(2, 3) += 0.01 * sin(alpha);
            world.GetCameraPos()(1, 3) += 0.01 * cos(alpha);
        }
        if (right) {
            is_frame_update_needed = true;
            world.GetCameraPos()(2, 3) -= 0.01 * sin(alpha);
            world.GetCameraPos()(1, 3) -= 0.01 * cos(alpha);
        }
        if (forward) {
            is_frame_update_needed = true;
            world.GetCameraPos()(1, 3) -= 0.01 * sin(alpha);
            world.GetCameraPos()(2, 3) += 0.01 * cos(alpha);
        }
        if (backward) {
            is_frame_update_needed = true;
            world.GetCameraPos()(1, 3) += 0.01 * sin(alpha);
            world.GetCameraPos()(2, 3) -= 0.01 * cos(alpha);
        }
        if (turn_clockwise) {
            is_frame_update_needed = true;
            world.GetCameraPos() *= rturn;
            alpha += std::numbers::pi / 180;
        }
        if (turn_counterclockwise) {
            is_frame_update_needed = true;
            world.GetCameraPos() *= lturn;
            alpha -= std::numbers::pi / 180;
        }

        if (is_frame_update_needed) {
            renderer.RenderFrame(pixels, world.GetCameraPos(), world.GetTriangles());
            texture.update(pixels);
            window.clear();
            window.draw(sprite);
            window.display();
        }
    }

    return 0;
}