#include "application.h"
#include "utility/except.h"

int main() {
    try {
        Renderer3D::Application app;
        app.Run();
    } catch (...) {
        Renderer3D::Except::react();
    }

    return 0;
}
