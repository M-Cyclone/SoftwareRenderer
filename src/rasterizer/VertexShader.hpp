#pragma once
#include <algorithm>
#include <cmath>
#include <functional>

#include <glm/glm.hpp>

#include "geometry/Vertex.h"
#include "utils/Utils.hpp"

struct VertexShader
{
    struct Output
    {
        glm::vec4 mvp_position;
        glm::vec3 mv_position;
        glm::vec3 mv_normal;
        glm::vec4 color;
        glm::vec4 light_space_pos;
        glm::vec2 texcoords;

        glm::vec3 view_light_pos;

        glm::vec3 tangent_space_light_pos;
        glm::vec3 tangent_space_view_pos;
        glm::vec3 tangent_space_frag_pos;
    };

    virtual Output operator()(const Vertex& vertex) const = 0;
};

struct VSMvp : public VertexShader
{
    glm::mat4 mat_model;
    glm::mat4 mat_view;
    glm::mat4 mat_proj;

    Output operator()(const Vertex& vertex) const override
    {
        glm::mat4 mat_view_model = mat_view * mat_model;
        glm::vec4 view_position =
            mat_view_model * glm::vec4(vertex.position, 1.0f);

        Output output{};
        output.mv_position  = view_position;
        output.mvp_position = mat_proj * view_position;
        output.mv_normal =
            glm::transpose(glm::inverse(glm::mat3(mat_view_model))) *
            vertex.normal;
        output.color     = vertex.basecolor;
        output.texcoords = vertex.texcoords;

        output.mv_position.z = -output.mv_position.z / k_max_real_depth;

        return output;
    }
};

struct VSMvpLight : public VertexShader
{
    glm::mat4 mat_model;
    glm::mat4 mat_view;
    glm::mat4 mat_proj;

    glm::mat4 mat_light_view;
    glm::mat4 mat_light_proj;

    Output operator()(const Vertex& vertex) const override
    {
        glm::mat4 mat_view_model = mat_view * mat_model;
        glm::vec4 view_position =
            mat_view_model * glm::vec4(vertex.position, 1.0f);

        Output output{};
        output.mv_position  = view_position;
        output.mvp_position = mat_proj * view_position;
        output.mv_normal =
            glm::transpose(glm::inverse(glm::mat3(mat_view_model))) *
            vertex.normal;
        output.color           = vertex.basecolor;
        output.light_space_pos = mat_light_proj * mat_light_view * mat_model *
                                 glm::vec4(vertex.position, 1.0f);
        output.texcoords = vertex.texcoords;

        output.mv_position.z = -output.mv_position.z / k_max_real_depth;

        return output;
    }
};

struct VSShadow : public VertexShader
{
    glm::mat4 mat_model;
    glm::mat4 mat_light_view;
    glm::mat4 mat_light_proj;

    Output operator()(const Vertex& vertex) const override
    {
        glm::vec4 view_position =
            mat_light_view * mat_model * glm::vec4(vertex.position, 1.0f);

        Output output{};
        output.mv_position  = view_position;
        output.mvp_position = mat_light_proj * view_position;

        output.mv_position.z = -output.mv_position.z / k_max_real_depth;

        return output;
    }
};

struct VSNormalMapping : public VertexShader
{
    glm::mat4 mat_model;
    glm::mat4 mat_view;
    glm::mat4 mat_proj;

    // World space.
    glm::vec3 light_pos;
    glm::vec3 view_pos;

    Output operator()(const Vertex& vertex) const override
    {
        glm::mat4 mv            = mat_view * mat_model;
        glm::vec4 view_position = mv * glm::vec4(vertex.position, 1.0f);

        Output output{};
        output.mv_position   = view_position;
        output.mv_position.z = -output.mv_position.z / k_max_real_depth;
        output.mvp_position  = mat_proj * view_position;

        output.mv_normal =
            glm::transpose(glm::inverse(glm::mat3(mv))) * vertex.normal;

        output.color     = vertex.basecolor;
        output.texcoords = vertex.texcoords;


        glm::mat3 inv_model =
            glm::transpose(glm::inverse(glm::mat3(mat_model)));

        glm::vec3 world_normal  = inv_model * vertex.normal;
        glm::vec3 world_tangent = inv_model * vertex.tangent;

        glm::mat3 tbn = glm::transpose(
            glm::mat3(world_tangent,
                      glm::normalize(glm::cross(world_normal, world_tangent)),
                      world_normal));

        output.tangent_space_light_pos = tbn * light_pos;
        output.tangent_space_view_pos  = tbn * view_pos;
        output.tangent_space_frag_pos =
            tbn * glm::vec3(mat_model * glm::vec4(vertex.position, 1.0f));

        return output;
    }
};