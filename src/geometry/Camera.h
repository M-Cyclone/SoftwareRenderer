#pragma once
#include <algorithm>
#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera
{
public:
    virtual ~Camera() noexcept = default;

    virtual glm::mat4 getProj() const = 0;
    virtual glm::mat4 getView() const = 0;
};

class FPSCamera : public Camera
{
public:
    FPSCamera(const glm::vec3& pos,
              float            yaw,
              float            pitch,
              float            fov,
              float            aspect_ratio,
              float            z_near,
              float            z_far);
    FPSCamera(const Camera&)            = delete;
    FPSCamera& operator=(const Camera&) = delete;
    virtual ~FPSCamera() noexcept       = default;

    glm::mat4 getProj() const override
    {
        return glm::perspective(
            glm::radians(m_fov), m_aspect_ratio, m_z_near, m_z_far);
    }
    glm::mat4 getView() const override
    {
        return glm::lookAt(m_position, m_position + m_forward, m_up);
    }

    glm::vec3 getPosition() const { return m_position; }

    void onCursorPos(double xpos, double ypos);
    void onMouseButton(int button, int action);

    void processKey(const glm::vec3& delta_move);

private:
    void update();

private:
    glm::vec3 m_position;

    float m_yaw;
    float m_pitch;
    float m_fov;
    float m_aspect_ratio;
    float m_z_near;
    float m_z_far;

    glm::vec3 m_forward;
    glm::vec3 m_up;
    glm::vec3 m_right;

    float m_move_speed   = 10.0f;
    float m_rotate_speed = 180.0f;

    float m_cursor_x;
    float m_cursor_y;
    bool  m_press_mouse_right = false;
};

class LightCamera : public Camera
{
public:
    LightCamera(const glm::vec3& pos,
                const glm::vec3& dir,
                float            half_width,
                float            half_height,
                float            z_near,
                float            z_far);
    LightCamera(const LightCamera&)            = delete;
    LightCamera& operator=(const LightCamera&) = delete;
    virtual ~LightCamera() noexcept            = default;

    glm::mat4 getProj() const override
    {
        return glm::ortho(-m_half_width,
                          m_half_width,
                          -m_half_height,
                          m_half_height,
                          m_z_near,
                          m_z_far);
    }
    glm::mat4 getView() const override
    {
        return glm::lookAt(m_position, m_position + m_forward, m_up);
    }

    void setState(const glm::vec3& pos, const glm::vec3& dir)
    {
        m_position = pos;
        m_forward  = dir;
        m_right =
            glm::normalize(glm::cross(m_forward, glm::vec3(0.0f, 1.0f, 0.0f)));
        m_up = glm::normalize(glm::cross(m_right, m_forward));
    }

private:
    glm::vec3 m_position;
    glm::vec3 m_forward;
    glm::vec3 m_up;
    glm::vec3 m_right;
    float     m_half_width;
    float     m_half_height;
    float     m_z_near;
    float     m_z_far;
};