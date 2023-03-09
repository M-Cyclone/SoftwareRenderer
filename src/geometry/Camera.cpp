#include "Camera.h"
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

static constexpr float k_pi         = 3.1415926535898f;
static constexpr float k_pi_div_180 = k_pi / 180.0f;

FPSCamera::FPSCamera(const glm::vec3& pos,
                     float            yaw,
                     float            pitch,
                     float            fov,
                     float            aspect_ratio,
                     float            z_near,
                     float            z_far)
    : m_position(pos)
    , m_yaw(yaw)
    , m_pitch(pitch)
    , m_fov(fov)
    , m_aspect_ratio(aspect_ratio)
    , m_z_near(z_near)
    , m_z_far(z_far)
{
    update();
}

void FPSCamera::update()
{
    float radius_yaw   = m_yaw * k_pi_div_180;
    float radius_pitch = m_pitch * k_pi_div_180;

    glm::vec3 front;
    front.x   = std::cos(radius_pitch) * std::cos(radius_yaw);
    front.y   = std::sin(radius_pitch);
    front.z   = std::cos(radius_pitch) * std::sin(radius_yaw);
    m_forward = glm::normalize(front);

    m_right =
        glm::normalize(glm::cross(m_forward, glm::vec3(0.0f, 1.0f, 0.0f)));
    m_up = glm::normalize(glm::cross(m_right, m_forward));
}

void FPSCamera::onCursorPos(double xpos, double ypos)
{
    if (m_press_mouse_right)
    {
        float dx = xpos - m_cursor_x;
        float dy = ypos - m_cursor_y;

        m_yaw += dx * m_rotate_speed;
        m_pitch -= dy * m_rotate_speed;

        m_pitch = std::clamp(m_pitch, -80.0f, 80.0f);
    }

    m_cursor_x = (float)xpos;
    m_cursor_y = (float)ypos;

    update();
}

void FPSCamera::onMouseButton(int button, int action)
{
    if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        m_press_mouse_right = (action == GLFW_PRESS);
    }
}

void FPSCamera::processKey(const glm::vec3& delta_move)
{
    if (!m_press_mouse_right)
    {
        return;
    }
    m_position += (delta_move.x * m_forward + delta_move.y * m_right +
                   delta_move.z * m_up) *
                  m_move_speed;
}


LightCamera::LightCamera(const glm::vec3& pos,
                         const glm::vec3& dir,
                         float            half_width,
                         float            half_height,
                         float            z_near,
                         float            z_far)
    : m_position(pos)
    , m_forward(glm::normalize(dir))
    , m_half_width(half_width)
    , m_half_height(half_height)
    , m_z_near(z_near)
    , m_z_far(z_far)
{
    m_right =
        glm::normalize(glm::cross(m_forward, glm::vec3(0.0f, 1.0f, 0.0f)));
    m_up = glm::normalize(glm::cross(m_right, m_forward));
}