#pragma once

#include "light.h"
#include "triangle.h"

namespace Renderer3D::Kernel {

struct SubObject;

class Object {

public:
    void PushTriangle(const Triangle& triangle);
    void PushPointLightSource(const PointLightSource& pls);
    void PushSpotLightSource(const SpotLightSource& sls);
    void PushSubObject(const AffineTransform& pos, Object&& obj);

    [[nodiscard]] const std::vector<Triangle>& Triangles() const;
    [[nodiscard]] const std::vector<SubObject>& Subobjects() const;
    [[nodiscard]] std::vector<SubObject>& Subobjects();
    [[nodiscard]] const std::vector<PointLightSource>& PointLightSources() const;
    [[nodiscard]] const std::vector<SpotLightSource>& SpotLightSources() const;

private:
    std::vector<Triangle> triangles_;
    std::vector<SubObject> subobjects_;
    std::vector<PointLightSource> point_light_sources_;
    std::vector<SpotLightSource> spot_light_sources_;
};

struct SubObject {
    AffineTransform pos;
    Object obj;
};

}  // namespace Renderer3D::Kernel
