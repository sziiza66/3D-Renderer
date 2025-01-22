#pragma once
#include <vector>
#include "position.h"
#include "triangle.h"

namespace Renderer3D::Kernel {

struct SubObject;

class Object {
public:
    void PushTriangle(Triangle triangle);

    void PushSubObject(Position pos, Object obj);

    [[nodiscard]] const std::vector<Triangle>& GetTriangles() const;

    [[nodiscard]] const std::vector<SubObject>& GetSubobjects() const;

private:
    std::vector<Triangle> triangles_;
    std::vector<SubObject> subobjects_;
};

// Вообще очень заманчиво сделать указатель на object в целях экономии памяти, не знаю, стоит ли.
struct SubObject {
    Position pos;
    Object obj;
};

}  // namespace Renderer3D::Kernel
