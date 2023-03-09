#include "DirectionalLight.h"

DirectionalLight::DirectionalLight(const glm::vec3& pos,
                                   const glm::vec3& target,
                                   float            half_width,
                                   float            half_height,
                                   float            z_near,
                                   float            z_far)
    : m_position(pos)
    , m_direction(glm::normalize(target - pos))
    , m_interesting_point(target)
{
    m_shadow_camera = std::make_shared<LightCamera>(
        m_position, m_direction, half_width, half_height, z_near, z_far);
}
