#include "Rasterizer.h"
#include <cmath>

#include <tbb/tbb.h>

#include <stb_image_write.h>

bool Rasterizer::init(const Desc& desc)
{
    m_width          = desc.width;
    m_height         = desc.height;
    m_draw_color     = desc.draw_color;
    m_draw_depth     = desc.draw_depth;
    m_enable_4x_msaa = desc.enable_4x_msaa;
    m_cull_mode      = desc.cull_model;

    // Always set the buffer size 4 times of the resolution, so that the msaa
    // won't reallocate memory.
    if (m_draw_color)
    {
        m_frame_buffer.resize(m_width * m_height * 4);
        m_render_result.resize(m_width * m_height);
    }
    if (m_draw_depth)
    {
        m_depth_buffer.resize(m_width * m_height * 4);
    }

    return true;
}

void Rasterizer::exit()
{}

void Rasterizer::saveImage() const
{
    std::vector<stbi_uc> image;
    for (const glm::vec4& v : m_render_result)
    {
        image.push_back(v.r * 255.9f);
        image.push_back(v.g * 255.9f);
        image.push_back(v.b * 255.9f);
        image.push_back(v.a * 255.9f);
    }

    stbi_flip_vertically_on_write(true);
    stbi_write_png("screen_shot.png", m_width, m_height, 4, image.data(), 0);
}

void Rasterizer::render(const std::vector<Vertex>& vertices,
                        const std::vector<size_t>& indices,
                        const VertexShader&        vert_shader,
                        const FragmentShader&      frag_shader)
{
    // Run vertex shader on each vertex.
    std::vector<VertexShader::Output> vertex_after_vs(vertices.size());
    for (size_t i = 0, n = vertices.size(); i < n; ++i)
    {
        vertex_after_vs[i] = vert_shader(vertices[i]);

        // Homo divide.
        float inv_w = 1.0f / vertex_after_vs[i].mvp_position.w;
        vertex_after_vs[i].mvp_position.x *= inv_w;
        vertex_after_vs[i].mvp_position.y *= inv_w;
        vertex_after_vs[i].mvp_position.z *= inv_w;

        // Change to view space.
        vertex_after_vs[i].mvp_position.x =
            (vertex_after_vs[i].mvp_position.x + 1.0f) * 0.5f * m_width;
        vertex_after_vs[i].mvp_position.y =
            (vertex_after_vs[i].mvp_position.y + 1.0f) * 0.5f * m_height;
        vertex_after_vs[i].mvp_position.z =
            (vertex_after_vs[i].mvp_position.z + 1.0f) * 0.5f;
        vertex_after_vs[i].mvp_position.w = inv_w;
    }

    // Triangle assemble.
    const size_t triangle_count = indices.size() / 3;
    for (size_t i = 0; i < triangle_count; ++i)
    {
        processTriangle(vertex_after_vs[indices[i * 3]],
                        vertex_after_vs[indices[i * 3 + 1]],
                        vertex_after_vs[indices[i * 3 + 2]],
                        frag_shader);
    }
}

void Rasterizer::processTriangle(const VertexShader::Output& v0,
                                 const VertexShader::Output& v1,
                                 const VertexShader::Output& v2,
                                 const FragmentShader&       frag_shader)
{
    // Triangle direction culling.
    glm::vec3 eye(0.0f, 0.0f, 0.0f);
    glm::vec3 p0(v0.mv_position);
    glm::vec3 p1(v1.mv_position);
    glm::vec3 p2(v2.mv_position);
    float     check_dir = glm::dot(glm::cross(p2 - p0, p1 - p0), eye - p0);

    //    2
    //   / \
	//  /   \
	// 1-----0
    // If the triangle's vertices are given along clockwise order, then the
    // normal direction should be the same as cr = cross(v02, v01), witch means
    // normal dot cr should be larger than 0.

    if (m_cull_mode == CullMode::All)
    {
        return;
    }

    if (m_cull_mode == CullMode::ClockWise && check_dir > 1e-4f)
    {
        return;
    }

    if (m_cull_mode == CullMode::CounterClockWise && check_dir < 1e-4)
    {
        return;
    }


    // Viewport transfromation.
    // x -> [0, width], y -> [0, height], z -> [0, 1]

    float x_min =
        getMin(v0.mvp_position.x, getMin(v1.mvp_position.x, v2.mvp_position.x));
    float x_max =
        getMax(v0.mvp_position.x, getMax(v1.mvp_position.x, v2.mvp_position.x));
    float y_min =
        getMin(v0.mvp_position.y, getMin(v1.mvp_position.y, v2.mvp_position.y));
    float y_max =
        getMax(v0.mvp_position.y, getMax(v1.mvp_position.y, v2.mvp_position.y));

    if (x_min > (float)m_width || x_max < 0.0f || y_min > (float)m_height ||
        y_max < 0.0f)
    {
        return;
    }


    // Rasterize triangle.
    const int x_lo = getMax((int)x_min, 0);
    const int x_hi = getMin((int)x_max, m_width - 1);
    const int y_lo = getMax((int)y_min, 0);
    const int y_hi = getMin((int)y_max, m_height - 1);

    if (m_enable_4x_msaa)
    {
        tbb::parallel_for(
            tbb::blocked_range<int>(y_lo, y_hi),
            [x_lo, x_hi, this, v0, v1, v2, &frag_shader](
                tbb::blocked_range<int> r)
            {
                for (int y = r.begin(); y <= r.end(); ++y)
                {
                    for (int x = x_lo; x <= x_hi; ++x)
                    {
                        // The pixel center's interpolating results.
                        // Used when the triangle covers the center.
                        auto center_param =
                            getInterpolateParam(x + 0.5f,
                                                y + 0.5f,
                                                v0.mvp_position,
                                                v1.mvp_position,
                                                v2.mvp_position);

                        bool done_fs =
                            false;  // The flag to assure fragment shader will
                                    // be call only once in one pixel.
                        glm::vec4 fs_color{};  // Record the fs result.
                        for (int i = 0; i < 4; ++i)
                        {
                            float curr_x = x + k_msaa_delta[i][0];
                            float curr_y = y + k_msaa_delta[i][1];

                            auto [alpha, beta, gamma] =
                                getInterpolateParam(curr_x,
                                                    curr_y,
                                                    v0.mvp_position,
                                                    v1.mvp_position,
                                                    v2.mvp_position);

                            // Check if is inside the triangle.
                            if (alpha < 0.0f || beta < 0.0f || gamma < 0.0f)
                                continue;

                            // Interpolate depth.
                            float z0_ = alpha / v0.mv_position.z;
                            float z1_ = beta / v1.mv_position.z;
                            float z2_ = gamma / v2.mv_position.z;
                            float zt  = 1.0f / (z0_ + z1_ + z2_);

                            size_t idx = ((y * m_width + x) << 2) + i;

                            // z-test.
                            if (zt >= m_depth_buffer[idx])
                            {
                                continue;
                            }
                            if (m_draw_depth)
                            {
                                m_depth_buffer[idx] = zt;
                            }

                            if (m_draw_color)
                            {
                                if (!done_fs)
                                {
                                    FragmentShader::Input input{};

                                    if (std::get<0>(center_param) < 0 ||
                                        std::get<1>(center_param) < 0 ||
                                        std::get<2>(center_param) < 0)
                                    {
                                        input.mv_position =
                                            interpolate(v0.mv_position,
                                                        v1.mv_position,
                                                        v2.mv_position,
                                                        z0_,
                                                        z1_,
                                                        z2_,
                                                        zt);
                                        input.mv_normal = glm::normalize(
                                            interpolate(v0.mv_normal,
                                                        v1.mv_normal,
                                                        v2.mv_normal,
                                                        z0_,
                                                        z1_,
                                                        z2_,
                                                        zt));
                                        input.light_space_pos =
                                            interpolate(v0.light_space_pos,
                                                        v1.light_space_pos,
                                                        v2.light_space_pos,
                                                        z0_,
                                                        z1_,
                                                        z2_,
                                                        zt);
                                        input.color = interpolate(v0.color,
                                                                  v1.color,
                                                                  v2.color,
                                                                  z0_,
                                                                  z1_,
                                                                  z2_,
                                                                  zt);
                                        input.texcoords =
                                            interpolate(v0.texcoords,
                                                        v1.texcoords,
                                                        v2.texcoords,
                                                        z0_,
                                                        z1_,
                                                        z2_,
                                                        zt);

                                        input.tangent_space_light_pos =
                                            interpolate(
                                                v0.tangent_space_light_pos,
                                                v1.tangent_space_light_pos,
                                                v2.tangent_space_light_pos,
                                                z0_,
                                                z1_,
                                                z2_,
                                                zt);
                                        input.tangent_space_view_pos =
                                            interpolate(
                                                v0.tangent_space_view_pos,
                                                v1.tangent_space_view_pos,
                                                v2.tangent_space_view_pos,
                                                z0_,
                                                z1_,
                                                z2_,
                                                zt);
                                        input.tangent_space_frag_pos =
                                            interpolate(
                                                v0.tangent_space_frag_pos,
                                                v1.tangent_space_frag_pos,
                                                v2.tangent_space_frag_pos,
                                                z0_,
                                                z1_,
                                                z2_,
                                                zt);
                                    }
                                    else
                                    {
                                        // Interpolate center's depth.
                                        float z0__ = std::get<0>(center_param) /
                                                     v0.mv_position.z;
                                        float z1__ = std::get<1>(center_param) /
                                                     v1.mv_position.z;
                                        float z2__ = std::get<2>(center_param) /
                                                     v2.mv_position.z;
                                        float zt_ = 1.0f / (z0__ + z1__ + z2__);

                                        input.mv_position =
                                            interpolate(v0.mv_position,
                                                        v1.mv_position,
                                                        v2.mv_position,
                                                        z0__,
                                                        z1__,
                                                        z2__,
                                                        zt_);
                                        input.mv_normal = glm::normalize(
                                            interpolate(v0.mv_normal,
                                                        v1.mv_normal,
                                                        v2.mv_normal,
                                                        z0__,
                                                        z1__,
                                                        z2__,
                                                        zt_));
                                        input.light_space_pos =
                                            interpolate(v0.light_space_pos,
                                                        v1.light_space_pos,
                                                        v2.light_space_pos,
                                                        z0__,
                                                        z1__,
                                                        z2__,
                                                        zt_);
                                        input.color = interpolate(v0.color,
                                                                  v1.color,
                                                                  v2.color,
                                                                  z0__,
                                                                  z1__,
                                                                  z2__,
                                                                  zt_);
                                        input.texcoords =
                                            interpolate(v0.texcoords,
                                                        v1.texcoords,
                                                        v2.texcoords,
                                                        z0__,
                                                        z1__,
                                                        z2__,
                                                        zt_);

                                        input.tangent_space_light_pos =
                                            interpolate(
                                                v0.tangent_space_light_pos,
                                                v1.tangent_space_light_pos,
                                                v2.tangent_space_light_pos,
                                                z0__,
                                                z1__,
                                                z2__,
                                                zt_);
                                        input.tangent_space_view_pos =
                                            interpolate(
                                                v0.tangent_space_view_pos,
                                                v1.tangent_space_view_pos,
                                                v2.tangent_space_view_pos,
                                                z0__,
                                                z1__,
                                                z2__,
                                                zt_);
                                        input.tangent_space_frag_pos =
                                            interpolate(
                                                v0.tangent_space_frag_pos,
                                                v1.tangent_space_frag_pos,
                                                v2.tangent_space_frag_pos,
                                                z0__,
                                                z1__,
                                                z2__,
                                                zt_);
                                    }

                                    fs_color = frag_shader(input);

                                    done_fs = true;
                                }

                                m_frame_buffer[idx] = fs_color;
                            }
                        }
                    }
                }
            });

        tbb::parallel_for(
            tbb::blocked_range<int>(y_lo, y_hi),
            [x_lo, x_hi, this](tbb::blocked_range<int> r)
            {
                for (int y = r.begin(); y <= r.end(); ++y)
                {
                    for (int x = x_lo; x <= x_hi; ++x)
                    {
                        size_t idx = (size_t(y * m_width + x) << 2);
                        m_render_result[idx >> 2] =
                            0.25f *
                            (m_frame_buffer[idx] + m_frame_buffer[idx + 1] +
                             m_frame_buffer[idx + 2] + m_frame_buffer[idx + 3]);
                    }
                }
            });
    }
    else
    {
        tbb::parallel_for(
            tbb::blocked_range<int>(y_lo, y_hi),
            [x_lo, x_hi, this, v0, v1, v2, &frag_shader](
                tbb::blocked_range<int> r)
            {
                for (int y = r.begin(); y <= r.end(); ++y)
                {
                    for (int x = x_lo; x <= x_hi; ++x)
                    {
                        if (!isInsideTriangle(x + 0.5f,
                                              y + 0.5f,
                                              v0.mvp_position,
                                              v1.mvp_position,
                                              v2.mvp_position))
                        {
                            continue;
                        }


                        // Interpolate alpha/beta/gamma.
                        auto params = getInterpolateParam(x + 0.5f,
                                                          y + 0.5f,
                                                          v0.mvp_position,
                                                          v1.mvp_position,
                                                          v2.mvp_position);

                        auto [alpha, beta, gamma] = params;


                        // Interpolate depth.
                        float z0_ = alpha / v0.mv_position.z;
                        float z1_ = beta / v1.mv_position.z;
                        float z2_ = gamma / v2.mv_position.z;
                        float zt  = 1.0f / (z0_ + z1_ + z2_);


                        // Depth test.
                        size_t idx = getIdx(x, y);
                        if (zt >= m_depth_buffer[idx])
                        {
                            continue;
                        }
                        if (m_draw_depth)
                        {
                            m_depth_buffer[idx] = zt;
                        }


                        // Calculate pixel.
                        if (m_draw_color)
                        {
                            FragmentShader::Input input{};
                            input.mv_position = interpolate(v0.mv_position,
                                                            v1.mv_position,
                                                            v2.mv_position,
                                                            z0_,
                                                            z1_,
                                                            z2_,
                                                            zt);
                            input.mv_normal =
                                glm::normalize(interpolate(v0.mv_normal,
                                                           v1.mv_normal,
                                                           v2.mv_normal,
                                                           z0_,
                                                           z1_,
                                                           z2_,
                                                           zt));
                            input.light_space_pos =
                                interpolate(v0.light_space_pos,
                                            v1.light_space_pos,
                                            v2.light_space_pos,
                                            z0_,
                                            z1_,
                                            z2_,
                                            zt);
                            input.color     = interpolate(v0.color,
                                                      v1.color,
                                                      v2.color,
                                                      z0_,
                                                      z1_,
                                                      z2_,
                                                      zt);
                            input.texcoords = interpolate(v0.texcoords,
                                                          v1.texcoords,
                                                          v2.texcoords,
                                                          z0_,
                                                          z1_,
                                                          z2_,
                                                          zt);

                            input.tangent_space_light_pos =
                                interpolate(v0.tangent_space_light_pos,
                                            v1.tangent_space_light_pos,
                                            v2.tangent_space_light_pos,
                                            z0_,
                                            z1_,
                                            z2_,
                                            zt);
                            ;
                            input.tangent_space_view_pos =
                                interpolate(v0.tangent_space_view_pos,
                                            v1.tangent_space_view_pos,
                                            v2.tangent_space_view_pos,
                                            z0_,
                                            z1_,
                                            z2_,
                                            zt);
                            ;
                            input.tangent_space_frag_pos =
                                interpolate(v0.tangent_space_frag_pos,
                                            v1.tangent_space_frag_pos,
                                            v2.tangent_space_frag_pos,
                                            z0_,
                                            z1_,
                                            z2_,
                                            zt);
                            ;

                            m_render_result[idx] = frag_shader(input);
                        }
                    }
                }
            });
    }
}
