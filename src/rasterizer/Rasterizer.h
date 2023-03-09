#pragma once
#include <memory>
#include <numeric>
#include <string>
#include <tuple>
#include <vector>

#include <glm/glm.hpp>

#include "FragmentShader.hpp"
#include "VertexShader.hpp"
#include "geometry/Camera.h"
#include "geometry/Vertex.h"
#include "utils/Utils.hpp"

class Rasterizer
{
public:
    enum class CullMode
    {
        None = 0,
        ClockWise,
        CounterClockWise,
        All
    };

    struct Desc
    {
        int width;
        int height;

        bool draw_color = true;
        bool draw_depth = true;

        bool enable_4x_msaa = false;

        CullMode cull_model = CullMode::None;
    };

private:
    static constexpr float k_msaa_delta[][2] = {
        {0.375f, 0.125f},
        {0.875f, 0.375f},
        {0.125f, 0.625f},
        {0.625f, 0.875f},
    };

public:
    Rasterizer()                             = default;
    Rasterizer(const Rasterizer&)            = delete;
    Rasterizer& operator=(const Rasterizer&) = delete;
    ~Rasterizer() noexcept                   = default;

    bool init(const Desc& desc);
    void exit();

    void clearFrameBuffer()
    {
        std::fill(m_frame_buffer.begin(),
                  m_frame_buffer.end(),
                  glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        std::fill(m_render_result.begin(),
                  m_render_result.end(),
                  glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    }
    void clearDepthBuffer()
    {
        std::fill(
            m_depth_buffer.begin(), m_depth_buffer.end(), k_max_relative_depth);
    }

    void setCullMode(CullMode mode) { m_cull_mode = mode; }
    void setEnable4xMsaa(bool val) { m_enable_4x_msaa = val; }

    const std::vector<glm::vec4>& getFramebuffer() const
    {
        return m_frame_buffer;
    }
    const std::vector<float>& getDepthBuffer() const { return m_depth_buffer; }

    const std::vector<glm::vec4>& getRenderResult() const
    {
        return m_render_result;
    }

    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }

    void render(const std::vector<Vertex>& vertices,
                const std::vector<size_t>& indices,
                const VertexShader&        vert_shader,
                const FragmentShader&      frag_shader);

    void saveImage() const;

private:
    size_t getIdx(int x, int y) const { return (y * m_width) + x; }

    void processTriangle(const VertexShader::Output& v0,
                         const VertexShader::Output& v1,
                         const VertexShader::Output& v2,
                         const FragmentShader&       frag_shader);

private:
    int  m_width          = 1280;
    int  m_height         = 720;
    bool m_draw_color     = true;
    bool m_draw_depth     = true;
    bool m_enable_4x_msaa = false;

    CullMode m_cull_mode = CullMode::None;

    std::vector<glm::vec4> m_frame_buffer;
    std::vector<float>     m_depth_buffer;
    std::vector<glm::vec4> m_render_result;
};