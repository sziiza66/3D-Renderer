#include "frame.h"

namespace Renderer3D::Kernel {

Frame::Frame() : data_(1) {
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
    data_.assign(data_.size(), {0, 0, 0});
}

const sf::Uint8* Frame::GetPixels() const {
    // Я выяснил, что ботлнек производительности был в том, что я делал лишнее копирование, когда передавал в функцию
    // sfml массив Uint8, так вот с этой функций я могу его передавать напрямую, без копирования.
    // Вот я честно не знаю, насколько это смертный грех, я помню, про какие подводные ками плюсов мы разговаривали,
    // если это так, то подскажи, пожалуйста, как это сделать лучше.
    return reinterpret_cast<const sf::Uint8*>(data_.data());
}

}  // namespace Renderer3D::Kernel
