#include "App.h"
#include <sstream>

#include <glm/glm.hpp>

#include "geometry/Vertex.h"

extern App::Desc* g_desc;

int App::run()
{
    // Initialization.
    if (!init(*g_desc))
    {
        return -1;
    }


    // The window handle system is based on GLFW, and the app will exit when the
    // window is destroyed.
    double last_time       = glfwGetTime();
    float  time_accumulate = 0.0f;
    while (!glfwWindowShouldClose(m_window))
    {
        double curr_time = glfwGetTime();
        float  dt        = float(curr_time - last_time);
        time_accumulate += dt;


        // The window just show a texture which is the soft rasterizer rendered.
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        update(dt);
        draw();

        present();

        glfwSwapBuffers(m_window);
        glfwPollEvents();


        // Show FPS. FPS will be shown at the window's title and refresh every 1
        // seconds.
        if (time_accumulate > 1.0f)
        {
            float              fps = 1.0f / dt;
            std::ostringstream oss;
            oss << m_title << " [FPS: " << fps << " ]";
            glfwSetWindowTitle(m_window, oss.str().c_str());
            time_accumulate -= 1.0f;
        }

        last_time = curr_time;
    }

    exit();

    return 0;
}

bool App::init(const Desc& desc)
{
    // App init.
    m_title      = desc.name;
    m_wnd_width  = desc.rasterizer_desc.width;
    m_wnd_height = desc.rasterizer_desc.height;


    // GLFW init.
    if (glfwInit() != GLFW_TRUE)
    {
        return false;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_window = glfwCreateWindow(
        m_wnd_width, m_wnd_height, m_title.c_str(), nullptr, nullptr);
    if (!m_window)
    {
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(m_window);


    // Glad init.
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        return false;
    }

    glViewport(0, 0, m_wnd_width, m_wnd_height);

    // Parameters setting.
    glfwSetWindowUserPointer(m_window, this);
    glfwSetMouseButtonCallback(
        m_window,
        [](GLFWwindow* window, int button, int action, int mods)
        {
            App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
            app->onMouseButton(button, action);
        });
    glfwSetCursorPosCallback(
        m_window,
        [](GLFWwindow* window, double xpos, double ypos)
        {
            App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
            app->onCursorPos(xpos, ypos);
        });


    // Use glfw for easier window handling.
    {
        float vertices[] = {
            1.0f,  1.0f,  0.0f, 1.0f, 1.0f,  // 右上
            1.0f,  -1.0f, 0.0f, 1.0f, 0.0f,  // 右下
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,  // 左下
            -1.0f, 1.0f,  0.0f, 0.0f, 1.0f   // 左上
        };

        uint32_t indices[] = { 0, 1, 2, 0, 2, 3 };

        // vao
        glCreateVertexArrays(1, &m_screen_vao);
        glBindVertexArray(m_screen_vao);

        // vbo
        glGenBuffers(1, &m_screen_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_screen_vbo);

        glBufferData(
            GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0,
                              3,
                              GL_FLOAT,
                              GL_FALSE,
                              5 * sizeof(float),
                              (void*)(0 * sizeof(float)));
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1,
                              2,
                              GL_FLOAT,
                              GL_FALSE,
                              5 * sizeof(float),
                              (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // ibo
        glGenBuffers(1, &m_screen_ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_screen_ibo);

        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glBindVertexArray(0);
    }


    // Create shader.
    {
        const char* vert_shader_src =
            "                                     \n"
            "#version 330 core                                              \n"
            "layout (location = 0) in vec3 a_pos;                           \n"
            "layout (location = 1) in vec2 a_texcoords;                     \n"
            "                                                               \n"
            "out vec2 v_texcoords;                                          \n"
            "                                                               \n"
            "void main()                                                    \n"
            "{                                                              \n"
            "    gl_Position = vec4(a_pos, 1.0);                            \n"
            "    v_texcoords = a_texcoords;                                 \n"
            "}                                                              \n"
            "";

        const char* frag_shader_src =
            "                                     \n"
            "#version 330 core                                              \n"
            "                                                               \n"
            "out vec4 frag_color;                                           \n"
            "                                                               \n"
            "in vec2 v_texcoords;                                           \n"
            "                                                               \n"
            "uniform sampler2D screen_tex;                                  \n"
            "                                                               \n"
            "void main()                                                    \n"
            "{                                                              \n"
            "    frag_color = texture(screen_tex, v_texcoords);             \n"
            "}                                                              \n"
            "";

        int success = GLFW_TRUE;

        uint32_t vert_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vert_shader, 1, &vert_shader_src, nullptr);
        glCompileShader(vert_shader);

        glGetShaderiv(vert_shader, GL_COMPILE_STATUS, &success);
        if (success == GLFW_FALSE)
        {
            return false;
        }

        uint32_t frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(frag_shader, 1, &frag_shader_src, nullptr);
        glCompileShader(frag_shader);

        glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &success);
        if (success == GLFW_FALSE)
        {
            return false;
        }


        m_screen_shader_program = glCreateProgram();
        glAttachShader(m_screen_shader_program, vert_shader);
        glAttachShader(m_screen_shader_program, frag_shader);
        glLinkProgram(m_screen_shader_program);

        glGetProgramiv(m_screen_shader_program, GL_LINK_STATUS, &success);
        if (success == GLFW_FALSE)
        {
            return false;
        }


        glDeleteShader(vert_shader);
        glDeleteShader(frag_shader);
    }


    // Create screen texture.
    {
        glGenTextures(1, &m_screen_tex);
        glBindTexture(GL_TEXTURE_2D, m_screen_tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_RGBA,
                     m_wnd_width,
                     m_wnd_height,
                     0,
                     GL_RGBA,
                     GL_FLOAT,
                     nullptr);
    }


    // Rasterizer init.
    // Scene renderer.
    if (!m_rasterizer.init(desc.rasterizer_desc))
    {
        return false;
    }

    // Light pass renderer.
    Rasterizer::Desc shadow_map_desc{};
    shadow_map_desc.width      = k_shadow_map_size;
    shadow_map_desc.height     = k_shadow_map_size;
    shadow_map_desc.draw_color = false;
    shadow_map_desc.draw_depth = true;
    shadow_map_desc.cull_model = Rasterizer::CullMode::CounterClockWise;
    if (!m_shadow_map.init(shadow_map_desc))
    {
        return false;
    }


    // Create cameras.
    // Used by scene renderer.
    m_render_camera = std::make_unique<FPSCamera>(
        glm::vec3(0.0f, 1.0f, 10.0f),
        -90.0f,
        0.0f,
        45.0f,
        (float)desc.rasterizer_desc.width / (float)desc.rasterizer_desc.height,
        1.0f,
        1000.0f);


    // Create directional light.
    m_light = std::make_unique<DirectionalLight>(glm::vec3(5.0f, 10.0f, 5.0f),
                                                 glm::vec3(0.0f, 2.0f, 0.0f),
                                                 10.0f,
                                                 10.0f,
                                                 0.1f,
                                                 k_max_real_depth);


    // Create scene objects.
    m_scene["plane"] =
        Plane::create(5.0f, 5.0f);  // The object that will render the shadow.
    m_scene["cube"] = Cube::create();  // The object that will cast the shadow.


    // Shader init.
    vs_light_pass = std::make_unique<VSShadow>();
    fs_light_pass = std::make_unique<FSShadow>();

    vs_mvp_with_light           = std::make_unique<VSMvpLight>();
    fs_pcss                     = std::make_unique<FSShadowPCSS>();
    fs_pcss->shadow_map_texture = m_shadow_map.getDepthBuffer().data();
    fs_pcss->shadow_map_width   = k_shadow_map_size;
    fs_pcss->shadow_map_height  = k_shadow_map_size;

    vs_normal_mapping = std::make_unique<VSNormalMapping>();
    fs_normal_mapping = std::make_unique<FSNormalMapping>(
        "../resources/brickwall.jpg",
        "../resources/brickwall_normal.jpg");  // Normal mapping fs needs 2
                                               // textures.


    return true;
}

void App::exit()
{
    // Clear resources.
    glfwDestroyWindow(m_window);
    m_window = nullptr;
    glfwTerminate();
}

void App::update(float dt)
{
    // Process input to camera.
    {
        glm::vec3 dir(0.0f, 0.0f, 0.0f);
        if (GLFW_PRESS == glfwGetKey(m_window, GLFW_KEY_W))
        {
            dir.x += 1.0f;
        }
        if (GLFW_PRESS == glfwGetKey(m_window, GLFW_KEY_S))
        {
            dir.x -= 1.0f;
        }
        if (GLFW_PRESS == glfwGetKey(m_window, GLFW_KEY_A))
        {
            dir.y -= 1.0f;
        }
        if (GLFW_PRESS == glfwGetKey(m_window, GLFW_KEY_D))
        {
            dir.y += 1.0f;
        }
        if (GLFW_PRESS == glfwGetKey(m_window, GLFW_KEY_Q))
        {
            dir.z -= 1.0f;
        }
        if (GLFW_PRESS == glfwGetKey(m_window, GLFW_KEY_E))
        {
            dir.z += 1.0f;
        }

        static constexpr float k_eps = 1e-4f;
        float                  len   = glm::length(dir);
        if (len > k_eps)
        {
            dir /= len;
        }
        dir *= dt;

        m_render_camera->processKey(dir);
    }


    // Other input handle.
    {
        // Screen shot.
        if (GLFW_PRESS == glfwGetKey(m_window, GLFW_KEY_P))
        {
            m_rasterizer.saveImage();
        }

        // Enable / disable 4xMsaa.
        {
            static int last_frame_key_m_state = 0;
            int        curr_state = glfwGetKey(m_window, GLFW_KEY_M);

            if (last_frame_key_m_state == GLFW_PRESS &&
                curr_state == GLFW_RELEASE)
            {
                static bool enable = true;
                enable             = !enable;
                m_rasterizer.setEnable4xMsaa(enable);
            }

            last_frame_key_m_state = curr_state;
        }
    }


    // Scene update.
    {
        // The plane is in xoy plane, which means it needs to rotate to xoz
        // plane.
        glm::mat4 model(1.0f);
        model = glm::rotate(
            model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        m_scene.at("plane")->setModel(
            model);  // Use "at()" to prevent accident instancing.
    }

    {
        static float angle = 0.0f;
        angle += 15.0f * dt;

        glm::mat4 model(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 1.8f, 0.0f));
        model = glm::rotate(
            model, glm::radians(angle), glm::vec3(1.0f, 5.0f, 6.0f));

        m_scene.at("cube")->setModel(model);
    }
}

void App::draw()
{
    // Clear buffer.
    m_shadow_map.clearDepthBuffer();

    m_rasterizer.clearFrameBuffer();
    m_rasterizer.clearDepthBuffer();


    // Prepare matrix data. Calculate here so that we don't need to calculate
    // mutiple times in one frame.
    glm::mat4 camera_proj = m_render_camera->getProj();
    glm::mat4 camera_view = m_render_camera->getView();

    glm::mat4 light_proj = m_light->getCamera()->getProj();
    glm::mat4 light_view = m_light->getCamera()->getView();


    // Draw shadow map.
    {
        vs_light_pass->mat_light_proj = light_proj;
        vs_light_pass->mat_light_view = light_view;

        for (const auto& [_, primitive] : m_scene)
        {
            vs_light_pass->mat_model = primitive->getModel();

            m_shadow_map.render(primitive->getVertices(),
                                primitive->getIndices(),
                                *vs_light_pass,
                                *fs_light_pass);
        }
    }


    // Draw scene.
    {
        {
            vs_mvp_with_light->mat_proj       = camera_proj;
            vs_mvp_with_light->mat_view       = camera_view;
            vs_mvp_with_light->mat_light_proj = light_proj;
            vs_mvp_with_light->mat_light_view = light_view;

            vs_mvp_with_light->mat_model = m_scene["plane"]->getModel();

            m_rasterizer.render(m_scene["plane"]->getVertices(),
                                m_scene["plane"]->getIndices(),
                                *vs_mvp_with_light,
                                *fs_pcss);
        }

        {
            vs_normal_mapping->mat_proj = camera_proj;
            vs_normal_mapping->mat_view = camera_view;

            vs_normal_mapping->light_pos = m_light->getPosition();
            vs_normal_mapping->view_pos  = m_render_camera->getPosition();

            vs_normal_mapping->mat_model = m_scene["cube"]->getModel();

            m_rasterizer.render(m_scene["cube"]->getVertices(),
                                m_scene["cube"]->getIndices(),
                                *vs_normal_mapping,
                                *fs_normal_mapping);
        }
    }
}

void App::present()
{
    glTexSubImage2D(GL_TEXTURE_2D,
                    0,
                    0,
                    0,
                    m_wnd_width,
                    m_wnd_height,
                    GL_RGBA,
                    GL_FLOAT,
                    m_rasterizer.getRenderResult().data());

    glBindVertexArray(m_screen_vao);
    glBindTexture(GL_TEXTURE_2D, m_screen_tex);
    glUseProgram(m_screen_shader_program);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}

void App::onCursorPos(double xpos, double ypos)
{
    m_render_camera->onCursorPos(xpos / m_wnd_width, ypos / m_wnd_height);
}

void App::onMouseButton(int button, int action)
{
    m_render_camera->onMouseButton(button, action);
}