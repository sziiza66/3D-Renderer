#pragma once

#include "object.h"

namespace Renderer3D::Kernel {

// По факту World это тот же самый Object, но который не умеет хранить треугольники, не знаю, правильно ли я сделал,
// может стоит вообще выкинуть класс мира, если я отделил от него камеру? Не знаю, звучит неразумно выкидывать World.
class World {

public:
    void PushObject(const AffineTransform& pos, Object&& obj);

    [[nodiscard]] const std::vector<SubObject>& Objects() const;

private:
    std::vector<SubObject> objects_;
};

}  // namespace Renderer3D::Kernel
