#include "world.h"

namespace Renderer3D::Kernel {

World::World() : ambient_light_(kDefaultAmbient) {
}

void World::PushObject(const AffineTransform& pos, Object&& obj) {
    objects_.emplace_back(pos, std::move(obj));
}

const std::vector<SubObject>& World::Objects() const {
    return objects_;
}

[[nodiscard]] const Color& World::AmbientLight() const {
    return ambient_light_;
}

}  // namespace Renderer3D::Kernel
