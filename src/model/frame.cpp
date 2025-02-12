#include "frame.h"

namespace Renderer3D::Kernel {

Frame::Frame() : data_(1), height_(1), width_(1) {
}

Frame::Frame(size_t height, size_t width)
    : data_((height == 0 ? 1 : height) * (width == 0 ? 1 : width)),
      height_((height == 0 ? 1 : height)),
      width_((width == 0 ? 1 : width)) {
}

Color& Frame::operator()(size_t x, size_t y) {
    return data_[x * width_ + y].color;
}

const Color& Frame::operator()(size_t x, size_t y) const {
    return data_[x * width_ + y].color;
}

size_t Frame::GetHeight() const {
    return height_;
}

size_t Frame::GetWidth() const {
    return width_;
}

void Frame::Clear() {
    data_.assign(data_.size(), {0, 0, 0, DefaultAlpha});
}

const Frame::ColorWithAlpha* Frame::GetPixels() const {
    return data_.data();
}

}  // namespace Renderer3D::Kernel
