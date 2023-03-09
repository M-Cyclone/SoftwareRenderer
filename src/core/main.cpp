#include "core/App.h"

App::Desc* g_desc = nullptr;

int main(int argc, char** argv)
{
    App::Desc desc{};
    desc.name                   = "software renderer";  // Window name.
    desc.rasterizer_desc.width  = 1280;                 // Window width.
    desc.rasterizer_desc.height = 720;                  // Window height.
    desc.rasterizer_desc.cull_model =
        Rasterizer::CullMode::CounterClockWise;  // Triangle cull mode.
    desc.rasterizer_desc.enable_4x_msaa = true;  // Default using 4xmsaa.

    g_desc = &desc;

    return App::getInstance().run();
}