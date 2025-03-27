#include "color.h"

#include <limits>

namespace Renderer3D::Kernel {

DiscreteColor MakeDiscrete(const Color& col) {
    return DiscreteColor(col.r * std::numeric_limits<uint8_t>::max(), col.g * std::numeric_limits<uint8_t>::max(),
                         col.b * std::numeric_limits<uint8_t>::max());
}

}  // namespace Renderer3D::Kernel
