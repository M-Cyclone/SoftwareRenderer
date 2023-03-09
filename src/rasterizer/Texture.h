#pragma once
#include <string>
#include <vector>

#include <stb_image.h>

#include <glm/glm.hpp>

class Texture
{
private:
    static constexpr float k_uc_to_float = 1.0f / 255.0f;

public:
    Texture(const std::string& path);
    Texture(const Texture&)            = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&&)                 = default;
    Texture& operator=(Texture&&)      = default;

    // u and v should be in [0, 1].
    glm::vec4 sample(float u, float v, int mipmap_level) const;

private:
    glm::vec4 sampleIner(int x, int y, int mipmap_level) const
    {
        assert(mipmap_level >= 0);
        assert(mipmap_level < 4);

        return m_data[mipmap_level][size_t(y * (m_width >> mipmap_level) + x)];
    }

private:
    int                    m_width  = 0;
    int                    m_height = 0;
    std::vector<glm::vec4> m_data[4];
};