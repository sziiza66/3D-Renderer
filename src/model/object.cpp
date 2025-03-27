#include "object.h"

namespace Renderer3D::Kernel {

void Object::PushTriangle(const Triangle& triangle) {
    triangles_.emplace_back(triangle);
}

void Object::PushPointLightSource(const PointLightSource& pls) {
    point_light_sources_.push_back(pls);
}

void Object::PushSubObject(const AffineTransform& pos, Object&& obj) {
    subobjects_.emplace_back(pos, std::move(obj));
}

const std::vector<Triangle>& Object::Triangles() const {
    return triangles_;
}

const std::vector<SubObject>& Object::Subobjects() const {
    return subobjects_;
}

const std::vector<PointLightSource>& Object::PointLightSources() const {
    return point_light_sources_;
}

}  // namespace Renderer3D::Kernel
