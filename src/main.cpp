#include "application.h"
#include "utility/cmd_parser.h"
#include "utility/except.h"

// #define NDEBUG

int main(int argc, char** argv) {
    try {
        auto [filename, scale] = Renderer3D::ParseCMD(argc, argv);
        if (!filename) {
            Renderer3D::PrintReference();
            return 0;
        }
        Renderer3D::Application app(filename, scale);
        app.Run();
    } catch (...) {
        Renderer3D::Except::react();
    }

    return 0;
}
