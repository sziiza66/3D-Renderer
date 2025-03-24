#include "zbuffer.h"

namespace Renderer3D::Kernel {

void ZBuffer::FitTo(const Frame& frame) {
    width_ = frame.GetWidth();
    buffer_.assign(frame.GetHeight() * width_, std::numeric_limits<double>::infinity());
}

const double& ZBuffer::operator()(size_t x, size_t y) const {
    return buffer_[x * width_ + y];
}
double& ZBuffer::operator()(size_t x, size_t y) {
    return buffer_[x * width_ + y];
}

}  // namespace Renderer3D::Kernel
