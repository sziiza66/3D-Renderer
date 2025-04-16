#pragma once

#include "frame.h"

namespace Renderer3D::Kernel {

class ZBuffer {
public:

    void FitTo(const Frame& frame);

    const double& operator()(size_t x, size_t y) const;
    double& operator()(size_t x, size_t y);

private:
    std::vector<double> buffer_;
    ssize_t width_ = 0;
};

}  // namespace Renderer3D::Kernel
