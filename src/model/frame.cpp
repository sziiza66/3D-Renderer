#include "frame.h"

namespace Renderer3D::Kernel {

Frame::Frame() : data_(1), height_(1), width_(1) {
}

Frame::Frame(ssize_t height, ssize_t width)
    : data_((height <= 0 ? 1 : height) * (width <= 0 ? 1 : width)),
      height_((height <= 0 ? 1 : height)),
      width_((width <= 0 ? 1 : width)) {
}

Color& Frame::operator()(ssize_t x, ssize_t y) {
    return data_[x * width_ + y].color;
}

const Color& Frame::operator()(ssize_t x, ssize_t y) const {
    return data_[x * width_ + y].color;
}

ssize_t Frame::GetHeight() const {
    return height_;
}

ssize_t Frame::GetWidth() const {
    return width_;
}

void Frame::Clear() {
    data_.assign(data_.size(), {0, 0, 0, kDefaultAlpha});
}

const Frame::ColorWithAlpha* Frame::GetPixels() const {
    return data_.data();
}

ssize_t Frame::CalcYDiscreteFromRealSegment(double y, double segment_length) const {
    return width_ * ((y + 1) / segment_length);
}

ssize_t Frame::CalcXDiscreteFromRealSegment(double x, double segment_length) const {
    return height_ * ((x + 1) / segment_length);
}

}  // namespace Renderer3D::Kernel
