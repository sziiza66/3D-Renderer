#include "rasterizer.h"

#include <cassert>

namespace Renderer3D::Kernel {
namespace {

constexpr double kSideOfTheCube = 2.0;

std::array<uint8_t, 3> GetVerticalOrderOfVertices(const Triangle& triangle) {
    std::array<uint8_t, 3> ret = {0, 1, 2};
    if (triangle.matrix(0, ret[2]) < triangle.matrix(0, ret[1])) {
        std::swap(ret[2], ret[1]);
    }
    if (triangle.matrix(0, ret[2]) < triangle.matrix(0, ret[0])) {
        std::swap(ret[2], ret[0]);
    }
    if (triangle.matrix(0, ret[1]) < triangle.matrix(0, ret[0])) {
        std::swap(ret[1], ret[0]);
    }
    return ret;
}

void FillSegment(const Triangle& triangle, Frame& frame, std::vector<double>& z_buffer_, ssize_t x, double real_y1,
                 double real_y2, double real_z, double z_diff_y, double real_z_diff_y) {

    ssize_t edge = frame.GetWidth() - 1;
    assert(edge >= 0);

    if (real_y1 < -1) {
        real_z += (-real_y1 - 1) * real_z_diff_y;
        real_y1 = -1;
    }

    // Есть непонятка, нужно обсудить.
    ssize_t y1 = frame.CalcYDiscreteFromRealSegment(real_y1, kSideOfTheCube);
    ssize_t y2 = frame.CalcYDiscreteFromRealSegment(real_y2, kSideOfTheCube);
    assert(y1 >= 0);
    assert(y2 >= 0);

    for (ssize_t y = y1; y <= (y2 >= edge ? edge : y2); ++y, real_z += z_diff_y) {
        if (real_z < z_buffer_[x * frame.GetWidth() + y]) {
            z_buffer_[x * frame.GetWidth() + y] = real_z;
            frame(x, y) = triangle.color;
        }
    }
}

void FillLowerTriangle(const Triangle& triangle, Frame& frame, std::vector<double>& z_buffer_, uint8_t lowest,
                       uint8_t middle, uint8_t highest, double real_z_diff_y, double z_diff_y, double z_diff_x,
                       double& real_x, double& real_z, ssize_t& x, double& prev_y) {
    double mid_x = (triangle.matrix(0, middle) <= 1 ? triangle.matrix(0, middle) : 1);
    double dx = kSideOfTheCube / frame.GetHeight();
    for (; real_x < mid_x; ++x, real_x += dx, real_z += z_diff_x) {
        // real_y1, real_y2 -- y координаты отрезка в видимом пространстве, который будет нарисован на экране.
        double real_y1 =
            (triangle.matrix(0, highest) == triangle.matrix(0, lowest)
                 ? triangle.matrix(1, highest)
                 : triangle.matrix(1, lowest) + (triangle.matrix(1, highest) - triangle.matrix(1, lowest)) *
                                                    (real_x - triangle.matrix(0, lowest)) /
                                                    (triangle.matrix(0, highest) - triangle.matrix(0, lowest)));
        double real_y2 =
            triangle.matrix(0, lowest) == triangle.matrix(0, middle)
                ? triangle.matrix(1, middle)
                : (triangle.matrix(1, lowest) + (triangle.matrix(1, middle) - triangle.matrix(1, lowest)) *
                                                    (real_x - triangle.matrix(0, lowest)) /
                                                    (triangle.matrix(0, middle) - triangle.matrix(0, lowest)));

        if (real_y1 > real_y2) {
            std::swap(real_y1, real_y2);
        }
        if (real_y2 < -1) {
            continue;
        }

        real_z += real_z_diff_y * (real_y1 - prev_y);
        prev_y = real_y1;

        // Рисуем этот отрезок
        FillSegment(triangle, frame, z_buffer_, x, real_y1, real_y2, real_z, z_diff_y, real_z_diff_y);
    }
}

void FillUpperTriangle(const Triangle& triangle, Frame& frame, std::vector<double>& z_buffer_, uint8_t lowest,
                       uint8_t middle, uint8_t highest, double real_z_diff_y, double z_diff_y, double z_diff_x,
                       double& real_x, double& real_z, ssize_t& x, double& prev_y) {
    double top_x = (triangle.matrix(0, highest) <= 1 ? triangle.matrix(0, highest) : 1);
    double dx = kSideOfTheCube / frame.GetHeight();
    for (; real_x < top_x; ++x, real_x += dx, real_z += z_diff_x) {
        // real_y1, real_y2 -- y координаты отрезка в видимом пространстве, который будет нарисован на экране.
        double real_y1 =
            (triangle.matrix(0, highest) == triangle.matrix(0, lowest)
                 ? triangle.matrix(1, highest)
                 : triangle.matrix(1, lowest) + (triangle.matrix(1, highest) - triangle.matrix(1, lowest)) *
                                                    (real_x - triangle.matrix(0, lowest)) /
                                                    (triangle.matrix(0, highest) - triangle.matrix(0, lowest)));
        double real_y2 =
            (triangle.matrix(0, highest) == triangle.matrix(0, middle)
                 ? triangle.matrix(1, highest)
                 : (triangle.matrix(1, middle) + (triangle.matrix(1, highest) - triangle.matrix(1, middle)) *
                                                     (real_x - triangle.matrix(0, middle)) /
                                                     (triangle.matrix(0, highest) - triangle.matrix(0, middle))));
        if (real_y1 > real_y2) {
            std::swap(real_y1, real_y2);
        }
        if (real_y2 < -1) {
            continue;
        }

        real_z += real_z_diff_y * (real_y1 - prev_y);
        prev_y = real_y1;

        // Рисуем этот отрезок
        FillSegment(triangle, frame, z_buffer_, x, real_y1, real_y2, real_z, z_diff_y, real_z_diff_y);
    }
}

void DrawTriangle(Frame& frame, std::vector<double>& z_buffer_, const Triangle& triangle) {

    // Индексы вершин треугольника, отсортированные по Ox.
    /*
        Я не изменил это место, т.к. невозможно вернуть 3 ссылки на 3 вектора, т.к. нельзя получить ссылку на
        вектор-столбец в матрице, потому что этого объекта нет, можно только получить новый вектор с помощью .col. В
        таком случае можно получить их копии, но числа lowest, middle, highest нужны дальше в коде, да, можно там везде
        вместо них начать использовать полученные вектора, может, ты на это и намекнул, просто я не совсем понял, почему
        так лучше. Будет меньше кода, то есть triangle.matrix(0, middle) заменится на middle_vector(0) или типа того,
        почему это действительно того стоит? И да, я поискал в Eigen что-то типа VectorView или какого-то умного
        враппера и нашел только Eigen::IndexedView, но мне кажется это слишком, + не факт, что оно может
        взаимодействовать с обычными векторами. Оставлю это место до ещё одного ревью или встречи.
    */
    auto [lowest, middle, highest] = GetVerticalOrderOfVertices(triangle);

    // Получаю векторы, напраленные из нижней (по Ox) точки треугольника в среднюю и верхнюю соотв.
    Vector3 v1 = triangle.matrix.col(middle).head(3) - triangle.matrix.col(lowest).head(3);
    Vector3 v2 = triangle.matrix.col(highest).head(3) - triangle.matrix.col(lowest).head(3);
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
    double z_diff_y = real_z_diff_y * kSideOfTheCube / frame.GetWidth();
    double z_diff_x = (v1(2) == 0 ? 0 : -v1(0) / v1(2) * kSideOfTheCube / frame.GetHeight());

    double real_x = (triangle.matrix(0, lowest) >= -1 ? triangle.matrix(0, lowest) : -1);
    double prev_y = triangle.matrix(1, lowest);

    double real_z =
        triangle.matrix(2, lowest) - (v1(2) == 0 ? 0 : v1(0) / v1(2)) * (real_x - triangle.matrix(0, lowest));

    ssize_t x = frame.CalcXDiscreteFromRealSegment(real_x, kSideOfTheCube);
    assert(x >= 0);

    // Треугольник разбивается на два других с одной из сторон параллельной Oy. Далее отриссовываем эти треугольники на
    // экране соотв. функциями.
    FillLowerTriangle(triangle, frame, z_buffer_, lowest, middle, highest, real_z_diff_y, z_diff_y, z_diff_x, real_x,
                      real_z, x, prev_y);

    FillUpperTriangle(triangle, frame, z_buffer_, lowest, middle, highest, real_z_diff_y, z_diff_y, z_diff_x, real_x,
                      real_z, x, prev_y);
}

}  // namespace

Frame BufferRasterizer::MakeFrame(const std::vector<Triangle>& triangles, Frame&& frame) {
    z_buffer_.assign(frame.GetHeight() * frame.GetWidth(), std::numeric_limits<double>::infinity());
    Frame ret(std::move(frame));
    ret.Clear();
    for (const auto& triangle : triangles) {
        // Т.к. я пока что не реализовал клиппинг, логика растерайзера обрабатывает случаи, когда вершины трегольника
        // выходят за границы видимой зоны, когда я реализую клиппинг, эту лишнюю логику можно будет убрать, ну или хотя
        // бы упростить.
        DrawTriangle(ret, z_buffer_, triangle);
    }
    return ret;
}

}  // namespace Renderer3D::Kernel
