#pragma once
#include "../model/entity.h"

namespace view {

class DotRasterizer {
public:
    DotRasterizer() : height_(1), width_(1) {
    }

    DotRasterizer(size_t width, size_t height) : height_(height), width_(width) {
    }

    void SetResolution(size_t width, size_t height);

    void operator()(sf::Uint8* frame, const entity::Triangle& triangle, const sf::Uint8* color) const;

    void Clear() {
    }

    size_t GetHeight() const {
        return height_;
    }

    size_t GetWidth() const {
        return width_;
    }

private:
    size_t height_;
    size_t width_;
};

class NoBufferRasterizer {
public:
    NoBufferRasterizer() : height_(1), width_(1) {
    }

    NoBufferRasterizer(size_t width, size_t height) : height_(height), width_(width) {
    }

    void SetResolution(size_t width, size_t height);

    void operator()(sf::Uint8* frame, const entity::Triangle& triangle, const sf::Uint8* color) const;

    void Clear() {
    }

    size_t GetHeight() const {
        return height_;
    }

    size_t GetWidth() const {
        return width_;
    }

private:
    size_t height_;
    size_t width_;
};

class BufferRasterizer {
public:
    BufferRasterizer() : height_(1), width_(1), z_buffer_(new double[1]) {
    }

    BufferRasterizer(size_t width, size_t height)
        : height_(height), width_(width), z_buffer_(new double[width * height]) {
    }

    ~BufferRasterizer() {
        delete[] z_buffer_;
    }

    void SetResolution(size_t width, size_t height);

    void operator()(sf::Uint8* frame, const entity::Triangle& triangle, const sf::Uint8* color) const;

    void Clear() {
        for (size_t i = 0; i < height_ * width_; ++i) {
            z_buffer_[i] = INFINITY;
        }
    }

    size_t GetHeight() const {
        return height_;
    }

    size_t GetWidth() const {
        return width_;
    }

private:
    double* z_buffer_;
    size_t height_;
    size_t width_;
};

}  // namespace view
