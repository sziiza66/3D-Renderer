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
    data_.assign(data_.size(), {0, 0, 0, DefaultAlpha});
}

const sf::Uint8* Frame::GetPixels() const {
    // Я выяснил, что ботлнек производительности был в том, что я делал лишнее копирование, когда передавал в функцию
    // sfml массив Uint8, так вот с этой функций я могу его передавать напрямую, без копирования.
    // Но это ещё не все, я попробовал сделать data_ вида std::vector<sf:Uint8> и сделать отдельную функцию, принимающую
    // Color и координаты пикселя, всё, чтобы не использовать этот каст, но рендер стал рабтать заметно медленнее. Это
    // техничеки не UB, как я понял, но если есть способ это убрать и оставить производительность, хотел бы использовать
    // его, подскажи, пожалуйста.
    // Я думаю, что если уж и придётся использовать каст, то его нужно отделить от интерфейса класса и оставить только в
    // том месте, где он нужен непосредственно в проекте, а здесь возвращать const ColorWithAlpha*, но пока оставлю так,
    // все равно нужно обсудить.
    return reinterpret_cast<const sf::Uint8*>(data_.data());
}

}  // namespace Renderer3D::Kernel
