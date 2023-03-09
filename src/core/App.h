#pragma once
#include <memory>
#include <string>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <unordered_map>

#include "geometry/Camera.h"
#include "geometry/DirectionalLight.h"
#include "geometry/Primitive.h"
#include "rasterizer/FragmentShader.hpp"
#include "rasterizer/Rasterizer.h"
#include "rasterizer/VertexShader.hpp"

class App
{
public:
    // Used for initialization.
    struct Desc
    {
        std::string      name;
        Rasterizer::Desc rasterizer_desc;
    };

private:
    // App class should only have one instance.
    App()                      = default;
    App(const App&)            = delete;
    App& operator=(const App&) = delete;
    ~App() noexcept            = default;

public:
    // Use lazy-mode singleton to get the only instance.
    static App& getInstance()
    {
        static App s_instance;
        return s_instance;
    }

    int run();

private:
    // Prepare all the data which rasterizer needs.
    bool init(const Desc& desc);
    void exit();

    // Scene update and input handling.
    void update(float dt);
    // Render / rasterizing the scene.
    void draw();

    // Show render result.
    void present();

    // Event callbacks.
    void onCursorPos(double xpos, double ypos);
    void onMouseButton(int button, int action);

private:
    std::string m_title = "mihoyo";

    // GLFW window, for easier window handling.
    GLFWwindow* m_window     = nullptr;
    int         m_wnd_width  = 1280;
    int         m_wnd_height = 720;

    // Using OpenGL to show the software rasterizer's result.
    uint32_t m_screen_vao            = 0;
    uint32_t m_screen_vbo            = 0;
    uint32_t m_screen_ibo            = 0;
    uint32_t m_screen_shader_program = 0;
    uint32_t m_screen_tex            = 0;

    // Scene.
    // Using unordered_map is not an efficient way to describe the scene,
    // but it's easier to understant which object is being rendered.
    std::unordered_map<std::string, std::shared_ptr<Primitive>> m_scene;

    // Real part to calculate the pixels.
    Rasterizer m_shadow_map;
    Rasterizer m_rasterizer;

    // Shaders.
    // Used in light pass.
    std::unique_ptr<VSShadow> vs_light_pass;
    std::unique_ptr<FSShadow> fs_light_pass;

    // Used by the plane which will show the cube's shadow.
    // Soft shadow algorithm is PCSS.
    std::unique_ptr<VSMvpLight>   vs_mvp_with_light;
    std::unique_ptr<FSShadowPCSS> fs_pcss;

    // Used by the cube, whose material is a brick wall with normal mapping.
    std::unique_ptr<VSNormalMapping> vs_normal_mapping;
    std::unique_ptr<FSNormalMapping> fs_normal_mapping;

    // Others.
    std::unique_ptr<FPSCamera> m_render_camera;  // The camera that observes the
                                                 // scene to render results.
    std::unique_ptr<DirectionalLight>
        m_light;  // Directional light with a inside camera to get view / proj
                  // matrix.
};