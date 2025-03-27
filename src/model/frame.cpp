#include "frame.h"

namespace Renderer3D::Kernel {

namespace {

ssize_t MakeAtLeastOne(ssize_t sz) {
    return sz <= 0 ? 1 : sz;
}

}  // namespace

Frame::Frame(enum UHeight height, enum UWidth width)
    : data_(MakeAtLeastOne(height) * MakeAtLeastOne(width)), width_(MakeAtLeastOne(width)) {
}

DiscreteColor& Frame::operator()(size_t x, size_t y) {
    return data_[x * width_ + y].color;
}

const DiscreteColor& Frame::operator()(size_t x, size_t y) const {
    return data_[x * width_ + y].color;
}

void Frame::FillWithBlackColor() {
    data_.assign(data_.size(), kWhite);
}

size_t Frame::Height() const {
    return data_.size() / (width_ == 0 ? 1 : width_);
}

size_t Frame::Width() const {
    return width_;
}

const Frame::DiscreteColorWithAlpha* Frame::Data() const {
    return data_.data();
}

}  // namespace Renderer3D::Kernel
