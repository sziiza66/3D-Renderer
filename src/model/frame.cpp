#include "frame.h"

namespace Renderer3D::Kernel {

namespace {

ssize_t MakeAtLeastOne(ssize_t sz) {
    return sz <= 0 ? 1 : sz;
}

} // namespace

Frame::Frame(FrameHeight height, FrameWidth width)
    : data_(MakeAtLeastOne(height) * MakeAtLeastOne(width)), width_(MakeAtLeastOne(width)) {
}

Color& Frame::operator()(size_t x, size_t y) {
    return data_[x * width_ + y].color;
}

const Color& Frame::operator()(size_t x, size_t y) const {
    return data_[x * width_ + y].color;
}

void Frame::FillWithBlackColor() {
    data_.assign(data_.size(), kWhite);
}

ssize_t Frame::GetHeight() const {
    return data_.size() / (width_ == 0 ? 1 : width_);
}

ssize_t Frame::GetWidth() const {
    return width_;
}

const Frame::ColorWithAlpha* Frame::Data() const {
    return data_.data();
}

}  // namespace Renderer3D::Kernel
