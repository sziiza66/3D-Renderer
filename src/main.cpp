
#include "model/camera.h"
#include "model/world.h"
#include "model/renderer.h"
#include "model/frame.h"

#include <iostream>
#include <memory>

int main() {

    constexpr size_t kWidth = 1800;
    constexpr size_t kHeight = 900;

    Renderer3D::Kernel::Camera camera(std::numbers::pi / 2, 0.1, 1000, static_cast<double>(kWidth) / kHeight);
    Renderer3D::Kernel::Position camera_pos;
    Renderer3D::Kernel::World world;
    Renderer3D::Kernel::Renderer renderer;

    Renderer3D::Kernel::Object obj;

    Renderer3D::Kernel::Triangle t1(Eigen::Matrix<double, 4, 3>{{0, 0.5, -0.5}, {0, 0.5, 0.5}, {0, 0.5, -0.5}, {1, 1, 1}}, Renderer3D::Kernel::Color{255, 0, 0});
    Renderer3D::Kernel::Triangle t2(Eigen::Matrix<double, 4, 3>{{0, 1, 0}, {0, 0, 1}, {0, 0, 0}, {1, 1, 1}}, Renderer3D::Kernel::Color{0, 255, 0});
    Renderer3D::Kernel::Triangle t3(Eigen::Matrix<double, 4, 3>{{0.4, 0.5, 0}, {1, -1, 0}, {1, 0.5, -4}, {1, 1, 1}}, Renderer3D::Kernel::Color{0, 0, 255});

    obj.PushTriangle(t1);
    obj.PushTriangle(t2);
    obj.PushTriangle(t3);
    Renderer3D::Kernel::Position pos;
    world.PushObject(pos, obj);

    sf::Texture texture;
    if (!texture.create(kWidth, kHeight)) {
        std::cout << "Unable to create texture.\n";
        return -1;
    }

    std::unique_ptr<sf::Uint8> pixels(new sf::Uint8[kWidth * kHeight * 4]);
    if (!pixels) {
        std::cout << "Unable to allocate frame.\n";
        return -1;
    }
    texture.update(pixels.get());

    sf::Sprite sprite;
    sprite.setTexture(texture);

    sf::RenderWindow window(sf::VideoMode(kWidth, kHeight), "3D Renderer");
    window.setKeyRepeatEnabled(false);

    Renderer3D::Kernel::Frame frame = renderer.RenderFrame(world.GetObjects(), camera_pos, camera, kHeight, kWidth);
    texture.update(pixels.get());
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
            camera_pos.DELETETHIS()(2, 3) += 0.01 * sin(alpha);
            camera_pos.DELETETHIS()(1, 3) += 0.01 * cos(alpha);
        }
        if (right) {
            is_frame_update_needed = true;
            camera_pos.DELETETHIS()(2, 3) -= 0.01 * sin(alpha);
            camera_pos.DELETETHIS()(1, 3) -= 0.01 * cos(alpha);
        }
        if (forward) {
            is_frame_update_needed = true;
            camera_pos.DELETETHIS()(1, 3) -= 0.01 * sin(alpha);
            camera_pos.DELETETHIS()(2, 3) += 0.01 * cos(alpha);
        }
        if (backward) {
            is_frame_update_needed = true;
            camera_pos.DELETETHIS()(1, 3) += 0.01 * sin(alpha);
            camera_pos.DELETETHIS()(2, 3) -= 0.01 * cos(alpha);
        }
        if (turn_clockwise) {
            is_frame_update_needed = true;
            camera_pos.DELETETHIS() *= rturn;
            alpha += std::numbers::pi / 180;
        }
        if (turn_counterclockwise) {
            is_frame_update_needed = true;
            camera_pos.DELETETHIS() *= lturn;
            alpha -= std::numbers::pi / 180;
        }

        if (is_frame_update_needed) {
            frame = renderer.RenderFrame(world.GetObjects(), camera_pos, camera, kHeight, kWidth);
            for (size_t i = 0; i < kHeight; ++i) {
                for (size_t j = 0; j < kWidth; ++j) {
                    pixels.get()[4 * (i * kWidth + j) + 0] = frame(i, j).r;
                    pixels.get()[4 * (i * kWidth + j) + 1] = frame(i, j).g;
                    pixels.get()[4 * (i * kWidth + j) + 2] = frame(i, j).b;
                    pixels.get()[4 * (i * kWidth + j) + 3] = 255;
                    // if (frame(i, j).r != 0 || frame(i, j).g != 0 || frame(i, j).b != 0) {
                    //     std::cout << "xdd\n";
                    // }
                }
            }
            texture.update(pixels.get());
            window.clear();
            window.draw(sprite);
            window.display();
        }
    }

    return 0;
}
