#pragma once

#include <SFML/Graphics.hpp>

#include "model/triangle.h"
#include "model/world.h"
#include "model/spectator.h"
#include "model/renderer.h"

namespace Renderer3D {

class Application {
    using Camera = Kernel::Camera;
    using Color = Kernel::Color;
    using Frame = Kernel::Frame;
    using Triangle = Kernel::Triangle;
    using Object = Kernel::Object;
    using PointLightSource = Kernel::PointLightSource;
    using SpotLightSource = Kernel::SpotLightSource;
    using DirectionalLightSource = Kernel::DirectionalLightSource;
    using Renderer = Kernel::Renderer;
    using Spectator = Kernel::Spectator;
    using World = Kernel::World;

    struct KeyHandlerAssociation {
        sf::Keyboard::Key key;
        void (Application::*handler)(bool is_key_pressed);
    };

public:
    Application(char* file_name, double scale);

    void Run();

private:
    void DrawFrame(const Frame& frame, const sf::Sprite& sprite, sf::Texture* texture);
    void HandleLoopIteration(const sf::Sprite& sprite, sf::Texture* texture);

private:
    static World PopulateWorld(char* file_name, double scale);

    void HandleUp(bool is_key_pressed);
    void HandleDown(bool is_key_pressed);
    void HandleLeft(bool is_key_pressed);
    void HandleRight(bool is_key_pressed);
    void HandleForward(bool is_key_pressed);
    void HandleBackward(bool is_key_pressed);
    void HandleTurnRight(bool is_key_pressed);
    void HandleTurnLeft(bool is_key_pressed);
    void HandlePointLight(bool is_key_pressed);
    void HandleSpotLight(bool is_key_pressed);
    void HandleToggleSun(bool is_key_pressed);

private:
    static constexpr size_t kDefaultWindowWidth = 1700;
    static constexpr size_t kDefaultWindowHeight = 1080;
    static constexpr double kSpectatorMovementSpeed = 0.2;
    static constexpr const char* kWindowName = "3D Renderer";
    static constexpr Color kDefaultDiffuseColor = {1, 1, 1};
    static constexpr Color kDefaultSpecularColor = {0.9, 0.8, 0.5};
    static constexpr Color kDefaultEmissionColor = {0, 0, 0};
    static constexpr uint32_t kDefaultSpecularPower = 50;
    static constexpr PointLightSource kDefaultPointLightSource = {{1, 0.8, 1}, 0.0001, 0.008, 0.2};
    static const SpotLightSource kDefaultSpotLightSource;
    static const DirectionalLightSource kDefaultDirectionalLightSource;

private:
    Spectator spectator_;
    sf::RenderWindow window_;
    Renderer renderer_;
    Frame frame_;
    World world_;
    //
    sf::Texture texture_;
    sf::Sprite sprite_;
    // Эта (эти) переменная нужна, чтобы каждый кард, пока зажата кнопка, не спанился новый источник света. Если бы static
    // переменные в функции-члене класса были бы уникальны тольк в рамках дной инстанции, было бы удобно держать её в
    // хендлерах.
    bool point_light_last_key_state_ = false;
    bool spot_light_last_key_state_ = false;
    bool dir_light_last_key_state_ = false;
};

}  // namespace Renderer3D
