#include "controller/application.h"
#include "utility/except.h"

int main() {

    try {
        Renderer3D::GUI::Application app;
        app.Run();
    } catch (...) {
        Renderer3D::Except::react();
    }

    return 0;
}
