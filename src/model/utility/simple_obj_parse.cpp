#include "simple_obj_parse.h"

namespace Renderer3D::Kernel {

namespace {

std::array<size_t, 3> ParseIndexes(const std::string& srt) {
    std::array<size_t, 3> ret = {0, 0, 0};
    uint8_t ind = 0;
    for (char c : srt) {
        if (ind > 2) {
            break;
        }
        if (c < '0' || c > '9') {
            ++ind;
            continue;
        }
        ret[ind] = ret[ind] * 10 + (c - '0');
    }
    return ret;
}

void Correct(double scale, double correction_eps, std::vector<Vector3>* vertices) {
    if (correction_eps <= 0) {
        return;
    }
    const Vector3& last = vertices->back();
    for (size_t i = 0; i + 1 < vertices->size(); ++i) {
        if ((last - (*vertices)[i]).norm() < correction_eps * scale) {
            (*vertices)[i] = last;
        }
    }
}

}  // namespace

Object ParseObj(std::ifstream& file, double correction_eps, const Color& diffuse_reflection_color,
                const Color& specular_reflection_color, uint32_t specular_power, double scale) {
    // Может парсить определенное подмножество .obj файлов, достает треугольники с их нрмалями.
    // Наличие других полигонов кроме треугольников приведёт преждевременному завершению функции, объект прочитается не
    // полностью и может быть кривым.
    assert(!std::isinf(scale));

    Object ret;
    std::vector<Vector3> vertices;
    std::vector<Vector3> normals;
    std::string type;
    while (file >> type) {
        if (type == "v") {
            vertices.emplace_back();
            file >> vertices.back()[0] >> vertices.back()[1] >> vertices.back()[2];
            vertices.back()[0] *= scale;
            vertices.back()[1] *= scale;
            vertices.back()[2] *= scale;
            Correct(scale, correction_eps, &vertices);
        } else if (type == "vn") {
            normals.emplace_back();
            file >> normals.back()[0] >> normals.back()[1] >> normals.back()[2];
        } else if (type == "f") {
            // v/vt/vn
            std::string vertex_info;
            Triangle new_triangle;
            new_triangle.diffuse_reflection_color = diffuse_reflection_color;
            new_triangle.specular_reflection_color = specular_reflection_color;
            new_triangle.specular_power = specular_power;
            for (uint8_t i = 0; i < 3; ++i) {
                file >> vertex_info;
                auto indexes = ParseIndexes(vertex_info);
                if (indexes[0] > 0 && indexes[0] <= vertices.size()) {
                    new_triangle.vertices.col(i) = vertices[indexes[0] - 1].homogeneous();
                } else {
                    new_triangle.vertices.col(i) = Vector4::UnitW();
                }
                if (indexes[2] > 0 && indexes[2] <= normals.size()) {
                    new_triangle.vertex_normals.col(i) = normals[indexes[2] - 1];
                } else {
                    new_triangle.vertex_normals.col(i) = Vector3{0, 0, 0};
                }
            }
            ret.PushTriangle(new_triangle);
        } else {
            continue;
        }
    }

    return ret;
}

}  // namespace Renderer3D::Kernel
