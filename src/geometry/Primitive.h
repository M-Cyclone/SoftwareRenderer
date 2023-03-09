#pragma once
#include <memory>
#include <utility>
#include <vector>

#include "Vertex.h"

class Primitive
{
public:
    virtual ~Primitive() noexcept = default;

    const std::vector<Vertex>& getVertices() const { return m_vertices; }
    const std::vector<size_t>& getIndices() const { return m_indices; }

    void             setModel(const glm::mat4& matrix) { m_model = matrix; }
    const glm::mat4& getModel() const { return m_model; }

protected:
    std::vector<Vertex> m_vertices;
    std::vector<size_t> m_indices;

    glm::mat4 m_model;
};

// Plane is in xoy plane.
class Plane : public Primitive
{
public:
    Plane(float scale_x, float scale_y, const glm::vec4& color);
    Plane(const Plane&)            = delete;
    Plane& operator=(const Plane&) = delete;
    virtual ~Plane() noexcept      = default;

    static std::shared_ptr<Primitive> create(
        float            scale_x = 1.0f,
        float            scale_y = 1.0f,
        const glm::vec4& color   = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f))
    {
        return std::shared_ptr<Plane>(new Plane(scale_x, scale_y, color));
    }
};

class Cube : public Primitive
{
public:
    Cube(float scale_x, float scale_y, float scale_z);
    Cube(const Cube&)            = delete;
    Cube& operator=(const Cube&) = delete;
    virtual ~Cube() noexcept     = default;

    static std::shared_ptr<Primitive> create(float scale_x = 1.0f,
                                             float scale_y = 1.0f,
                                             float scale_z = 1.0f)
    {
        return std::shared_ptr<Cube>(new Cube(scale_x, scale_y, scale_z));
    }
};