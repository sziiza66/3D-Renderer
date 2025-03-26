#pragma once

#include "triangle.h"

namespace Renderer3D::Kernel {

struct SubObject;

class Object {

public:
    void PushTriangle(Triangle triangle);
    void PushSubObject(const AffineTransform& pos, Object&& obj);

    [[nodiscard]] const std::vector<Triangle>& GetTriangles() const;
    [[nodiscard]] const std::vector<SubObject>& GetSubobjects() const;

private:
    std::vector<Triangle> triangles_;
    std::vector<SubObject> subobjects_;
};

struct SubObject {
    AffineTransform pos;
    Object obj;
};

}  // namespace Renderer3D::Kernel
