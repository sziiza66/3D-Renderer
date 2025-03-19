#include "world.h"

namespace Renderer3D::Kernel {

void World::PushObject(Matrix4 pos, Object obj) {
    objects_.emplace_back(std::move(pos), std::move(obj));
}

const std::vector<SubObject>& World::GetObjects() const {
    return objects_;
}

}  // namespace Renderer3D::Kernel
