#include "object.h"

namespace Renderer3D::Kernel {

void Object::PushTriangle(Triangle triangle) {
    triangles_.emplace_back(std::move(triangle));
}

void Object::PushSubObject(Matrix4 pos, Object obj) {
    subobjects_.emplace_back(std::move(pos), std::move(obj));
}

const std::vector<Triangle>& Object::GetTriangles() const {
    return triangles_;
}

const std::vector<SubObject>& Object::GetSubobjects() const {
    return subobjects_;
}

}  // namespace Renderer3D::Kernel
