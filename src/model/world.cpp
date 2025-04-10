#include "world.h"

namespace Renderer3D::Kernel {

World::World() : ambient_light_(kDefaultAmbient) {
}

void World::PushObject(const AffineTransform& pos, Object&& obj) {
    objects_.emplace_back(pos, std::move(obj));
}

void World::PushDirectionalLightSource(const DirectionalLightSource& dls) {
    dir_light_sources_.push_back(dls);
}

// Тупенький метод, да вообще всё здесь тупенькое.
void World::PopDirectionalLightSource() {
    dir_light_sources_.pop_back();
}

const std::vector<SubObject>& World::Objects() const {
    return objects_;
}

const Color& World::AmbientLight() const {
    return ambient_light_;
}

const std::vector<DirectionalLightSource>& World::DirectionalLightSources() const {
    return dir_light_sources_;
}

}  // namespace Renderer3D::Kernel
