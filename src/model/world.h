#pragma once

#include "object.h"

namespace Renderer3D::Kernel {

// По факту World это тот же самый Object, но который не умеет хранить треугольники, не знаю, правильно ли я сделал,
// может стоит вообще выкинуть класс мира, если я отделил от него камеру? Не знаю, звучит неразумно выкидывать World.
class World {
public:
    World();

    void PushObject(const AffineTransform& pos, Object&& obj);
    void PushDirectionalLightSource(const DirectionalLightSource& dls);
    void PopDirectionalLightSource();

    [[nodiscard]] const std::vector<SubObject>& Objects() const;
    [[nodiscard]] const Color& AmbientLight() const;
    [[nodiscard]] const std::vector<DirectionalLightSource>& DirectionalLightSources() const;

private:
    constexpr static Color kDefaultAmbient = {0.1, 0.1, 0.1};

private:
    std::vector<SubObject> objects_;
    std::vector<DirectionalLightSource> dir_light_sources_;
    Color ambient_light_;
};

}  // namespace Renderer3D::Kernel
