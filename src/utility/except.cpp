#include "except.h"

#include <exception>
#include <iostream>

namespace Renderer3D::Except {
void react() {
    try {
        throw;
    } catch (std::exception& e) {
        std::cout << "Program crashed with error: " << e.what() << '\n';
    } catch (...) {
        std::cout << "Program crashed because someone is dumb.\n";
    }
}
}  // namespace Renderer3D::Except
