#include "Texture.h"
#include <algorithm>
#include <cassert>

Texture::Texture(const std::string& path)
{
    int      channel{};
    stbi_uc* data =
        stbi_load(path.c_str(), &m_width, &m_height, &channel, STBI_rgb_alpha);
    if (!data)
    {
        std::string new_path = "../" + path;
        data                 = stbi_load(
            new_path.c_str(), &m_width, &m_height, &channel, STBI_rgb_alpha);
    }
    assert(data != nullptr);

    m_data[0].resize(m_width * m_height);
    for (int x = 0; x < m_width; ++x)
    {
        for (int y = 0; y < m_height; ++y)
        {
            size_t idx(y * m_height + x);
            m_data[0][idx] = glm::vec4(data[(idx << 2)],
                                       data[(idx << 2) + 1],
                                       data[(idx << 2) + 2],
                                       data[(idx << 2) + 3]) *
                             k_uc_to_float;
        }
    }

    stbi_image_free(data);


    // Generate mipmap.
    for (int i = 1; i < 4; ++i)
    {
        int width  = (m_width >> i);
        int height = (m_height >> i);

        m_data[i].resize(width * height);

        for (int x = 0; x < width; ++x)
        {
            for (int y = 0; y < height; ++y)
            {
                int last_x     = (x << 1);
                int last_y     = (y << 1);
                int last_width = (width << 1);

                m_data[i][y * width + x] =
                    (m_data[i - 1][last_width * (last_y) + last_x] +
                     m_data[i - 1][last_width * (last_y + 1) + last_x] +
                     m_data[i - 1][last_width * (last_y) + last_x + 1] +
                     m_data[i - 1][last_width * (last_y + 1) + last_x + 1]) *
                    0.25f;
            }
        }
    }
}

glm::vec4 Texture::sample(float u, float v, int mipmap_level) const
{
    int width  = (m_width >> mipmap_level);
    int height = (m_height >> mipmap_level);

    u *= width;
    v *= height;

    int x_lo = u;
    int y_lo = v;
    x_lo     = std::clamp(x_lo, 0, width - 1);
    y_lo     = std::clamp(y_lo, 0, height - 1);
    // return sampleIner(x_lo, y_lo);

    int x_hi = std::clamp(x_lo + 1, 0, width - 1);
    int y_hi = std::clamp(y_lo + 1, 0, height - 1);

    float delta_u = u - x_lo - 0.5f;
    float delta_v = v - y_lo - 0.5f;

    if (delta_u < 0.0f) delta_u += 1.0f;
    if (delta_v < 0.0f) delta_v += 1.0f;

    float d_u_rest = 1.0f - delta_u;
    float d_v_rest = 1.0f - delta_v;

    return d_u_rest * d_v_rest * sampleIner(x_lo, y_lo, mipmap_level) +
           d_u_rest * delta_v * sampleIner(x_lo, y_hi, mipmap_level) +
           delta_u * delta_v * sampleIner(x_hi, y_hi, mipmap_level) +
           delta_u * d_v_rest * sampleIner(x_hi, y_lo, mipmap_level);
}
