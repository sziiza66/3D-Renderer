#include "world.h"

namespace Renderer3D::Kernel {

void World::PushObject(const HomoTransform& pos, Object&& obj) {
    objects_.emplace_back(pos, std::move(obj));
}

const std::vector<SubObject>& World::GetObjects() const {
    return objects_;
}

}  // namespace Renderer3D::Kernel
