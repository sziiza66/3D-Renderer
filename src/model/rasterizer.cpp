#include "rasterizer.h"

#include <cassert>

#include "zbuffer.h"

namespace Renderer3D::Kernel {
namespace {

constexpr double kSideOfTheCube = 2.0;

auto GetVerticalOrderOfVertices(const Triangle& triangle, TriMatrix& preserved) {
    std::array<uint8_t, 3> ret = {0, 1, 2};

    // Eigen::Block не swap'ается :(
    if (triangle.vertices(0, ret[2]) < triangle.vertices(0, ret[1])) {
        std::swap(ret[2], ret[1]);
    }
    if (triangle.vertices(0, ret[2]) < triangle.vertices(0, ret[0])) {
        std::swap(ret[2], ret[0]);
    }
    if (triangle.vertices(0, ret[1]) < triangle.vertices(0, ret[0])) {
        std::swap(ret[1], ret[0]);
    }

    return std::tuple<ConstVertexRef, ConstVertexRef, ConstVertexRef, VertexRef, VertexRef, VertexRef>{
        triangle.vertices.col(ret[0]), triangle.vertices.col(ret[1]), triangle.vertices.col(ret[2]),
        preserved.col(ret[0]),         preserved.col(ret[1]),         preserved.col(ret[2])};
}

Color CalculateLightIntensityColor(const PointLightSource& source, double d2) {
    return source.color / (source.k_const + source.k_linear * std::sqrt(d2) + source.k_quadr * d2);
}

DiscreteColor CalculateColorOfPizxel(const Color& col, const Vector3& inter_coords, const Vector3& normal,
                                     const Color& ambient, const std::vector<PLSInSpace>& pls) {
    Color ret = col;
    Color modulator = ambient;
    for (const PLSInSpace& pl : pls) {
        double dotprod = normal.dot((inter_coords - pl.position).normalized());
        modulator += (dotprod >= 0 ? dotprod : 0) *
                     CalculateLightIntensityColor(pl.source_data, (inter_coords - pl.position).squaredNorm());
    }

    return MakeDiscrete(ret);
}

Vector3 InverseInterpolation(const Vector3& coords) {
    double z = 1.0 / coords(2);
    return {z * coords(0), z * coords(1), z};
}

void FillSegment(const Color& col, const Vector3& normal, const Color& ambient, const std::vector<PLSInSpace>& pls,
                 size_t x, const Vector3& inter_coords_1, const Vector3& inter_coords_2, double real_y1, double real_y2,
                 double real_z, double z_diff_y, double real_z_diff_y, Frame* frame, ZBuffer* z_buffer_) {
    Vector3 inter_coords = inter_coords_1;
    Vector3 inter_coords_inc = (inter_coords_2 - inter_coords_1) / (real_y2 - real_y1);
    if (real_y1 < -1) {
        inter_coords += inter_coords_inc * (-real_y1 - 1);
        real_z += (-real_y1 - 1) * real_z_diff_y;
        real_y1 = -1;
    }

    size_t y1 = ((real_y1 + 1) / kSideOfTheCube) * frame->Width();
    size_t y2 = ((real_y2 + 1) / kSideOfTheCube) * frame->Width() + 1;
    assert(y1 < y2);

    size_t edge = (y2 >= frame->Width() ? frame->Width() : y2);

    for (size_t y = y1; y != (y2 >= edge ? edge : y2); ++y, real_z += z_diff_y) {
        if (real_z < (*z_buffer_)(x, y)) {
            (*z_buffer_)(x, y) = real_z;
            (*frame)(x, y) = CalculateColorOfPizxel(col, InverseInterpolation(inter_coords), normal, ambient, pls);
        }
        inter_coords += inter_coords_inc * kSideOfTheCube / frame->Width();
    }
}

void FillLowerTriangle(const Triangle& triangle, const Vector3& normal, const Color& ambient,
                       const std::vector<PLSInSpace>& pls, ConstVertexRef lowest, ConstVertexRef middle,
                       ConstVertexRef highest, Vector3 lowest_p, Vector3 middle_p, Vector3 highest_p,
                       double real_z_diff_y, double z_diff_y, double z_diff_x, Frame* frame, ZBuffer* z_buffer_,
                       double* real_x, double* real_z, size_t* x, double* prev_y) {
    double mid_x = (middle(0) <= 1 ? middle(0) : 1);
    double dx = kSideOfTheCube / frame->Height();
    for (; *real_x < mid_x; ++(*x), *real_x += dx, *real_z += z_diff_x) {
        // real_y1, real_y2 -- y координаты отрезка в видимом пространстве, который будет нарисован на экране.
        double real_y1 = highest(0) == lowest(0)
                             ? highest(1)
                             : lowest(1) + (highest(1) - lowest(1)) * (*real_x - lowest(0)) / (highest(0) - lowest(0));
        double real_y2 = lowest(0) == middle(0)
                             ? middle(1)
                             : lowest(1) + (middle(1) - lowest(1)) * (*real_x - lowest(0)) / (middle(0) - lowest(0));

        if ((real_y2 < -1 && real_y1 < -1) || (real_y1 > 1 && real_y2 > 1)) {
            continue;
        }

        // Интерполяция данных
        // --------------------
        Vector3 inter_coords_1 = highest(0) == lowest(0) ? highest_p
                                                         : lowest_p + (highest_p - lowest_p) * (*real_x - lowest(0)) /
                                                                          (highest(0) - lowest(0));
        Vector3 inter_coords_2 =
            middle(0) == lowest(0) ? middle_p
                                   : lowest_p + (middle_p - lowest_p) * (*real_x - middle(0)) / (middle(0) - lowest(0));
        // --------------------

        if (real_y1 > real_y2) {
            std::swap(real_y1, real_y2);
            // Данные тоже нужно свапнуть
            std::swap(inter_coords_1, inter_coords_2);
        }

        *real_z += real_z_diff_y * (real_y1 - *prev_y);
        *prev_y = real_y1;

        // Рисуем этот отрезок
        FillSegment(triangle.color, normal, ambient, pls, *x, inter_coords_1, inter_coords_2, real_y1, real_y2, *real_z,
                    z_diff_y, real_z_diff_y, frame, z_buffer_);
    }
}

void FillUpperTriangle(const Triangle& triangle, const Vector3& normal, const Color& ambient,
                       const std::vector<PLSInSpace>& pls, ConstVertexRef lowest, ConstVertexRef middle,
                       ConstVertexRef highest, Vector3 lowest_p, Vector3 middle_p, Vector3 highest_p,
                       double real_z_diff_y, double z_diff_y, double z_diff_x, Frame* frame, ZBuffer* z_buffer_,
                       double* real_x, double* real_z, size_t* x, double* prev_y) {
    double top_x = (highest(0) <= 1 ? highest(0) : 1);
    double dx = kSideOfTheCube / frame->Height();
    for (; *real_x < top_x; ++(*x), *real_x += dx, *real_z += z_diff_x) {
        // real_y1, real_y2 -- y координаты отрезка в видимом пространстве, который будет нарисован на экране.
        double real_y1 = highest(0) == lowest(0)
                             ? highest(1)
                             : lowest(1) + (highest(1) - lowest(1)) * (*real_x - lowest(0)) / (highest(0) - lowest(0));
        double real_y2 = highest(0) == middle(0)
                             ? highest(1)
                             : middle(1) + (highest(1) - middle(1)) * (*real_x - middle(0)) / (highest(0) - middle(0));

        if ((real_y2 < -1 && real_y1 < -1) || (real_y1 > 1 && real_y2 > 1)) {
            continue;
        }

        // Интерполяция данных
        // --------------------
        Vector3 inter_coords_1 = highest(0) == lowest(0) ? highest_p
                                                         : lowest_p + (highest_p - lowest_p) * (*real_x - lowest(0)) /
                                                                          (highest(0) - lowest(0));
        Vector3 inter_coords_2 = highest(0) == middle(0) ? highest_p
                                                         : middle_p + (highest_p - middle_p) * (*real_x - middle(0)) /
                                                                          (highest(0) - middle(0));
        // --------------------

        if (real_y1 > real_y2) {
            std::swap(real_y1, real_y2);
            // Данные тоже нужно свапнуть
            std::swap(inter_coords_1, inter_coords_2);
        }

        *real_z += real_z_diff_y * (real_y1 - *prev_y);
        *prev_y = real_y1;

        // Рисуем этот отрезок
        FillSegment(triangle.color, normal, ambient, pls, *x, inter_coords_1, inter_coords_2, real_y1, real_y2, *real_z,
                    z_diff_y, real_z_diff_y, frame, z_buffer_);
    }
}

void DrawTriangle(const Triangle& triangle, TriMatrix preserved, const Vector3& normal, const Color& ambient,
                  const std::vector<PLSInSpace>& pls, Frame* frame, ZBuffer* z_buffer_) {
    // Eigen::Block'и вершин треугольника, отсортированные по Ox.
    auto [lowest, middle, highest, lowest_p, middle_p, highest_p] = GetVerticalOrderOfVertices(triangle, preserved);

    /*
        Вроде это единственное место, где я конвертирую Vector4 -> Vector3 или наоборот, если реально стоит выделить
        конвертацию в функцию, то без проблем. В любом случае я сделаю read/write сцены в файл, а в файле я буду хранить
        матрицы компактно, так что наверное что-то такое понадобится.
    */
    // Получаю векторы, напраленные из нижней (по Ox) точки треугольника в среднюю и верхнюю соотв.
    Vector3 v1 = middle.head(3) - lowest.head(3);
    Vector3 v2 = highest.head(3) - lowest.head(3);
    // Получаю вектр нормали к плоскости треугольника.
    v1 = v1.cross(v2);

    // Смысл происходящего далее такой: я поддерживаю две точки, соответствующие друг другу, точку на треугольнике в
    // нашем видимом пространстве и точку треугольника на экране. Я двигаю точку на экране по оси Ox и Oy, при этом в
    // нужной пропорции изменяя координату точки в видимом пространстве с помощью посчитанных здесь коэффициентов.
    // real_z_diff_y -- Коэффициент изменения координаты z при изменении координаты y (т.е. такое, чтобы снова попасть в
    // плоскость треугольника) в видимом пространстве.
    // z_diff_y -- Коэффициент изменения координаты z при изменении координаты y на такое значение, чтобы кордината y на
    // экране сдвинулась на один пиксель.
    // z_diff_x -- аналогично для x.
    // real_x, real_y, real_z -- координаты поддреживаемой точки в видимом пространстве.
    double real_z_diff_y = (v1(2) == 0 ? 0 : -v1(1) / v1(2));
    double z_diff_y = real_z_diff_y * kSideOfTheCube / frame->Width();
    double z_diff_x = (v1(2) == 0 ? 0 : -v1(0) / v1(2) * kSideOfTheCube / frame->Height());

    // Я передаю эти четыре переменные по указателю как изменяемые входные данные. Я мб позже сменю это на return tuple
    // или struct, мне нужно подумать.
    double real_x = (lowest(0) >= -1 ? lowest(0) : -1);
    double prev_y = lowest(1);
    double real_z = lowest(2) - (v1(2) == 0 ? 0 : v1(0) / v1(2)) * (real_x - lowest(0));
    size_t x = frame->Height() * ((real_x + 1) / kSideOfTheCube);
    x = (x >= frame->Height() ? frame->Height() - 1 : x);

    // Интерполяция данных
    // --------------------
    assert(lowest_p(2) != 0);
    assert(middle_p(2) != 0);
    assert(highest_p(2) != 0);
    lowest_p(2) = 1.0 / lowest_p(2);
    middle_p(2) = 1.0 / middle_p(2);
    highest_p(2) = 1.0 / highest_p(2);

    lowest_p(0) *= lowest_p(2);
    lowest_p(1) *= lowest_p(2);
    middle_p(0) *= middle_p(2);
    middle_p(1) *= middle_p(2);
    highest_p(0) *= highest_p(2);
    highest_p(1) *= highest_p(2);
    // --------------------

    // Треугольник разбивается на два других с одной из сторон параллельной Oy. Далее отриссовываем эти треугольники на
    // экране соотв. функциями.
    // Я не уверен, нужно ли всё-таки менять название функции, теперь видно, что меняется?
    FillLowerTriangle(triangle, normal, ambient, pls, lowest, middle, highest, lowest_p.head(3), middle_p.head(3),
                      highest_p.head(3), real_z_diff_y, z_diff_y, z_diff_x, frame, z_buffer_, &real_x, &real_z, &x,
                      &prev_y);

    FillUpperTriangle(triangle, normal, ambient, pls, lowest, middle, highest, lowest_p.head(3), middle_p.head(3),
                      highest_p.head(3), real_z_diff_y, z_diff_y, z_diff_x, frame, z_buffer_, &real_x, &real_z, &x,
                      &prev_y);
}

}  // namespace

Frame BufferRasterizer::MakeFrame(const std::vector<Triangle>& triangles, const std::vector<TriMatrix>& preserved,
                                  const std::vector<PLSInSpace>& pls, const std::vector<Vector3>& normals,
                                  const Color& ambient, Frame&& frame) {
    assert(triangles.size() == normals.size());
    Frame ret(std::move(frame));
    z_buffer_.FitTo(ret);
    ret.FillWithBlackColor();
    // for (const auto& triangle : triangles) {
    //     DrawTriangle(triangle, pls, normals, &ret, &z_buffer_);
    // }
    for (size_t i = 0; i < triangles.size(); ++i) {
        DrawTriangle(triangles[i], preserved[i], normals[i], ambient, pls, &ret, &z_buffer_);
    }
    return ret;
}

}  // namespace Renderer3D::Kernel
