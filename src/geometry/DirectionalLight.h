#pragma once
#include <memory>

#include <glm/glm.hpp>

#include "Camera.h"

class DirectionalLight
{
public:
    DirectionalLight(const glm::vec3& pos,
                     const glm::vec3& target,
                     float            half_width,
                     float            half_height,
                     float            z_near,
                     float            z_far);
    DirectionalLight(const DirectionalLight&)            = delete;
    DirectionalLight& operator=(const DirectionalLight&) = delete;

    void setPosition(const glm::vec3& pos)
    {
        m_position          = pos;
        m_interesting_point = m_position + m_direction;
        m_shadow_camera->setState(m_position, m_direction);
    }
    void setDirection(const glm::vec3& dir)
    {
        m_direction         = glm::normalize(dir);
        m_interesting_point = m_position + m_direction;
        m_shadow_camera->setState(m_position, m_direction);
    }
    void setInterestingPoint(const glm::vec3& point)
    {
        m_interesting_point = point;
        m_direction         = glm::normalize(m_interesting_point - m_position);
        m_shadow_camera->setState(m_position, m_direction);
    }

    glm::vec3 getPosition() const { return m_position; }

    std::shared_ptr<LightCamera> getCamera() const { return m_shadow_camera; }

private:
    glm::vec3 m_position;
    glm::vec3 m_direction;
    glm::vec3 m_interesting_point;

    std::shared_ptr<LightCamera> m_shadow_camera;
};