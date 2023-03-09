#pragma once

#include <glm/glm.hpp>

static constexpr float k_max_float          = std::numeric_limits<float>::max();
static constexpr float k_max_relative_depth = 1.0f;
static constexpr float k_max_real_depth     = 50.0f;
static constexpr int   k_shadow_map_size    = 512;

// If you want to use this, set the texturn to this and set shadow map size
// to 8.
static constexpr float k_test_texture[] = {
    0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1,
    0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0,
    1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0,
};

static glm::vec3 convertNormal(const glm::vec3& normal, const glm::mat4& mv_inv)
{
    auto trans_normal = mv_inv * glm::vec4(normal, 0.0f);
    return glm::vec3(trans_normal);
}

static auto getInterpolateParam(float            x,
                                float            y,
                                const glm::vec4& p0,
                                const glm::vec4& p1,
                                const glm::vec4& p2)
    -> std::tuple<float, float, float>
{
    float c0 =
        (x * (p1.y - p2.y) + (p2.x - p1.x) * y + p1.x * p2.y - p2.x * p1.y) /
        (p0.x * (p1.y - p2.y) + (p2.x - p1.x) * p0.y + p1.x * p2.y -
         p2.x * p1.y);

    float c1 =
        (x * (p2.y - p0.y) + (p0.x - p2.x) * y + p2.x * p0.y - p0.x * p2.y) /
        (p1.x * (p2.y - p0.y) + (p0.x - p2.x) * p1.y + p2.x * p0.y -
         p0.x * p2.y);

    float c2 = 1.0f - c0 - c1;

    return { c0, c1, c2 };
}

template <typename T>
static T interpolate(const T& v0,
                     const T& v1,
                     const T& v2,
                     float    z0_,
                     float    z1_,
                     float    z2_,
                     float    zt)
{
    return (z0_ * v0 + z1_ * v1 + z2_ * v2) * zt;
}

static float vec2Cross(const glm::vec2& v1, const glm::vec2& v2)
{
    return v1.x * v2.y - v1.y * v2.x;
}

static bool isInsideTriangle(float            x,
                             float            y,
                             const glm::vec4& v0,
                             const glm::vec4& v1,
                             const glm::vec4& v2)
{
    glm::vec2 center(x, y);
    glm::vec2 v0_(v0);
    glm::vec2 v1_(v1);
    glm::vec2 v2_(v2);
    float     a = vec2Cross(v0_ - center, v1_ - center);
    float     b = vec2Cross(v1_ - center, v2_ - center);
    float     c = vec2Cross(v2_ - center, v0_ - center);

    return (a >= 0 && b >= 0 && c >= 0) || (a <= 0 && b <= 0 && c <= 0);
}

template <typename T>
static const T& getMin(const T& a, const T& b)
{
    return (a < b) ? a : b;
}

template <typename T>
static const T& getMax(const T& a, const T& b)
{
    return (a > b) ? a : b;
}