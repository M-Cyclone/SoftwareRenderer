#pragma once
#include <algorithm>
#include <cmath>
#include <functional>
#include <random>

#include <glm/glm.hpp>

#include "geometry/Vertex.h"
#include "rasterizer/Texture.h"
#include "utils/Utils.hpp"

struct FragmentShader
{
    struct Input
    {
        glm::vec3 mv_position;
        glm::vec3 mv_normal;
        glm::vec4 color;
        glm::vec4 light_space_pos;
        glm::vec2 texcoords;

        glm::vec3 tangent_space_light_pos;
        glm::vec3 tangent_space_view_pos;
        glm::vec3 tangent_space_frag_pos;
    };

    virtual glm::vec4 operator()(const Input& input) const = 0;
};

// Simply show the default color.
struct FSFlat : public FragmentShader
{
    glm::vec4 operator()(const Input& input) const override
    {
        return input.color;
    }
};

// Used in the shadow pass. FS will do nothing and the work is done by depth
// buffer.
struct FSShadow : public FragmentShader
{
    int          shadow_map_width;
    int          shadow_map_height;
    const float* shadow_map_texture = nullptr;

    glm::vec4 operator()(const Input& input) const override
    {
        return glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    }

protected:
    float sampleShadowMap(glm::vec2 texcoords) const
    {
        int x = shadow_map_width * texcoords.x;
        int y = shadow_map_height * texcoords.y;

        x = std::clamp(x, 0, shadow_map_width - 1);
        y = std::clamp(y, 0, shadow_map_height - 1);

        int index = y * shadow_map_width + x;
        return shadow_map_texture[index];
    }
};

// Use for shadow map debug. This shader will use the shadow map as a texture.
struct FSShadowDebug : public FSShadow
{
    glm::vec4 operator()(const Input& input) const override
    {
        float depth = sampleShadowMap(input.texcoords);
        return glm::vec4(depth, depth, depth, 1.0f);
    }
};

// The shader that will use shadow map to calculate the shadow area.
struct FSShadowDraw : public FSShadow
{
    glm::vec4 operator()(const Input& input) const override
    {
        glm::vec3 proj_coords =
            glm::vec3(input.light_space_pos) / input.light_space_pos.w;
        proj_coords = proj_coords * 0.5f + glm::vec3(0.5f, 0.5f, 0.5f);

        float current_depth = proj_coords.z;
        float texture_depth = sampleShadowMap(glm::vec2(proj_coords));

        float bias         = 0.05f;
        bool  is_in_shadow = (current_depth > texture_depth + bias);

        float color = (is_in_shadow ? 0.0f : 1.0f);

        return glm::vec4(color, color, color, 1.0f);
    }
};

struct FSShadowPCSS : public FSShadow
{
    glm::vec3 view_light_pos;

    glm::vec4 operator()(const Input& input) const override
    {
        glm::vec3 proj_coords =
            glm::vec3(input.light_space_pos) / input.light_space_pos.w;
        proj_coords = proj_coords * 0.5f + glm::vec3(0.5f, 0.5f, 0.5f);


        auto [block_average_depth, block_count] =
            blockSearch(glm::vec2(proj_coords), proj_coords.z);
        if (block_count == 0)
        {
            return input.color;
        }
        // return glm::vec4(block_average_depth, block_average_depth,
        // block_average_depth, 1.0f);
        float penumbra_size =
            getPenumbraSize(proj_coords.z, block_average_depth);
        float color = calculatePCF(proj_coords, penumbra_size);


        return glm::vec4(color, color, color, 1.0f);
    }

private:
    static constexpr int   k_ring_count   = 10;
    static constexpr int   k_sample_count = 10;
    static constexpr float k_PI           = 3.14159265359f;
    static constexpr float k_2_PI         = 2.0f * k_PI;
    static constexpr float k_angle_step =
        k_2_PI * (float)k_ring_count / (float)k_sample_count;
    static constexpr float k_inv_sample_count = 1.0f / (float)k_sample_count;
    static constexpr float k_radius_step      = k_inv_sample_count;

    mutable glm::vec2 sample_disk[k_sample_count];

    void getPoisssonSamples() const
    {
        static std::default_random_engine            e;
        static std::uniform_real_distribution<float> r(0.0f, 1.0f);

        float angle  = r(e) * k_2_PI;
        float radius = k_inv_sample_count;

        for (int i = 0; i < k_sample_count; ++i)
        {
            sample_disk[i] = glm::vec2(glm::cos(angle), glm::sin(angle)) *
                             std::pow(radius, 0.75f);
            radius += k_radius_step;
            angle += k_angle_step;
        }
    }


    static constexpr float k_block_radius = 5.0f;

    std::pair<float, int> blockSearch(glm::vec2 texcoords,
                                      float     current_depth) const
    {
        getPoisssonSamples();

        float block_average_depth = 0.0f;
        int   block_count         = 0;
        bool  is_blocked          = false;
        for (int i = 0; i < k_sample_count; ++i)
        {
            int u = texcoords.x * shadow_map_width +
                    sample_disk[i].x * k_block_radius;
            int v = texcoords.y * shadow_map_height +
                    sample_disk[i].y * k_block_radius;
            if (u < 0 || u >= shadow_map_width || v < 0 ||
                v >= shadow_map_height)
            {
                continue;
            }

            float texture_depth = shadow_map_texture[v * shadow_map_width + u];
            if (current_depth + 0.01f > texture_depth)
            {
                block_average_depth += texture_depth;
                ++block_count;
                is_blocked = true;
            }
        }

        return { block_average_depth / (float)block_count, block_count };
    }


    static constexpr float k_light_width = 10.0f;

    float getPenumbraSize(float receiver_depth, float block_depth) const
    {
        return k_light_width * std::max(receiver_depth - block_depth, 0.0f) /
               block_depth;
    }


    float calculatePCF(glm::vec3 shadow_texcoords, float penumbra_size) const
    {
        getPoisssonSamples();

        float visibility        = 0.0f;
        int   real_sample_count = 0;
        for (int i = 0; i < k_sample_count; ++i)
        {
            int u = shadow_texcoords.x * shadow_map_width +
                    sample_disk[i].x * penumbra_size;
            int v = shadow_texcoords.y * shadow_map_height +
                    sample_disk[i].y * penumbra_size;

            if (u < 0 || u >= shadow_map_width || v < 0 ||
                v >= shadow_map_height)
            {
                continue;
            }
            ++real_sample_count;

            float texture_depth = shadow_map_texture[v * shadow_map_width + u];

            visibility +=
                (texture_depth + 0.02f > shadow_texcoords.z) ? 1.0f : 0.0f;
        }

        return visibility / (float)real_sample_count;
    }
};

struct FSShowTexture : public FragmentShader
{
    Texture texture;

    FSShowTexture(const std::string& path) : texture(path) {}
    glm::vec4 operator()(const Input& input) const override
    {
        return texture.sample(input.texcoords.x, input.texcoords.y, 1);
    }
};

struct FSNormalMapping : public FragmentShader
{
    Texture diffuse_tex;
    Texture normal_tex;

    FSNormalMapping(const std::string& diffuse_path,
                    const std::string& normal_path)
        : diffuse_tex(diffuse_path), normal_tex(normal_path)
    {}
    glm::vec4 operator()(const Input& input) const override
    {
        // Prepare.
        glm::vec3 color =
            diffuse_tex.sample(input.texcoords.x, input.texcoords.y, 1);
        glm::vec3 normal = normal_tex.sample(
            input.texcoords.x, input.texcoords.y, 1);  // tangent space
        normal = normal * 2.0f - glm::vec3(1.0f, 1.0f, 1.0f);

        glm::vec3 light_dir = glm::normalize(input.tangent_space_light_pos -
                                             input.tangent_space_frag_pos);
        glm::vec3 view_dir  = glm::normalize(input.tangent_space_view_pos -
                                            input.tangent_space_frag_pos);

        float     light_dot_normal = glm::dot(light_dir, normal);
        glm::vec3 reflect_dir = 2.0f * light_dot_normal * normal - light_dir;

        glm::vec3 half_way = glm::normalize(light_dir + view_dir);


        // Ambient.
        glm::vec3 ambient = color * 0.1f;


        // Diffuse.
        glm::vec3 diffuse = std::max(light_dot_normal, 0.0f) * color;


        // Specular.
        glm::vec3 specular =
            glm::vec3(0.2f, 0.2f, 0.2f) *
            std::pow(std::max(glm::dot(normal, half_way), 0.0f), 32.0f);


        return glm::vec4(ambient + diffuse + specular, 1.0f);
    }
};