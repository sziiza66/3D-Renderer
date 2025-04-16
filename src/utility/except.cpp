#include "except.h"

#include <exception>
#include <iostream>

namespace Renderer3D::Except {
void react() {
    try {
        throw;
    } catch (std::invalid_argument& e) {
        std::cout << "Program crashed with error: " << e.what() << '\n';
        std::cout << "Make sure scale value for .obj is valid\n";
    } catch (std::out_of_range& e) {
        std::cout << "Program crashed with error: " << e.what() << '\n';
        std::cout << "Make sure scale value for .obj is valid\n";
    } catch (std::exception& e) {
        std::cout << "Program crashed with error: " << e.what() << '\n';
    } catch (...) {
        std::cout << "Program crashed because someone is dumb.\n";
    }
}
}  // namespace Renderer3D::Except
