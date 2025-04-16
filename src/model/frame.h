#pragma once

#include <cstddef>
#include <sys/types.h>
#include <vector>

#include "color.h"

namespace Renderer3D::Kernel {

class Frame {
    // Нужно из-за специфики sfml.
    struct DiscreteColorWithAlpha {
        DiscreteColor color;
        uint8_t alpha = kDefaultAlpha;

        static constexpr uint8_t kDefaultAlpha = 255;
    };

public:
    // Приписка S нужна, чтобы компилятор не путал енамы с функциями Height и Width, хотя я могу в некоторых случаях
    // обозначить перед именем енама 'enum', и это помогает, но это работает не во всех случаях.
    enum SWidth : ssize_t;
    enum SHeight : ssize_t;

    Frame() = default;
    Frame(SHeight height, SWidth width);

    DiscreteColor& operator()(ssize_t x, ssize_t y);
    const DiscreteColor& operator()(ssize_t x, ssize_t y) const;

    void FillWithBlackColor();
    [[nodiscard]] size_t Height() const;
    [[nodiscard]] size_t Width() const;
    [[nodiscard]] const DiscreteColorWithAlpha* Data() const;

private:
    static constexpr DiscreteColorWithAlpha kWhite = {0, 0, 0, DiscreteColorWithAlpha::kDefaultAlpha};

private:
    std::vector<DiscreteColorWithAlpha> data_;
    ssize_t width_ = 0;
};

}  // namespace Renderer3D::Kernel
