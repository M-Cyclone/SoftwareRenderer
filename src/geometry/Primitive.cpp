#include "Primitive.h"

/*
 *   struct Vertex
 *	{
 *		glm::vec3 position;
 *		glm::vec3 normal;
 *		glm::vec3 tangent;
 *		glm::vec4 basecolor;
 *		glm::vec2 texcoords;
 *	};
 */

Plane::Plane(float scale_x, float scale_y, const glm::vec4& color)
{
    // 3___________2
    //  |         |
    //  |         |
    //  |         |
    //  |         |
    // 0|_________|1
    m_vertices = std::vector<Vertex>{
        Vertex{glm::vec3(-scale_x, -scale_y, 0.0f),
               glm::vec3(0.0f, 0.0f, 1.0f),
               glm::vec3(1.0f, 0.0f, 0.0f),
               color, glm::vec2(0.0f, 0.0f)},
        Vertex{glm::vec3(+scale_x, -scale_y, 0.0f),
               glm::vec3(0.0f, 0.0f, 1.0f),
               glm::vec3(1.0f, 0.0f, 0.0f),
               color, glm::vec2(1.0f, 0.0f)},
        Vertex{glm::vec3(+scale_x, +scale_y, 0.0f),
               glm::vec3(0.0f, 0.0f, 1.0f),
               glm::vec3(1.0f, 0.0f, 0.0f),
               color, glm::vec2(1.0f, 1.0f)},
        Vertex{glm::vec3(-scale_x, +scale_y, 0.0f),
               glm::vec3(0.0f, 0.0f, 1.0f),
               glm::vec3(1.0f, 0.0f, 0.0f),
               color, glm::vec2(0.0f, 1.0f)},
    };

    m_indices = std::vector<size_t>{ 0, 1, 2, 0, 2, 3 };
}

Cube::Cube(float scale_x, float scale_y, float scale_z)
{
    //    1________ 2        y+
    //    /|      /|         ^
    //   /_|_____/ |         |
    //  5|0|_ _ 6|_|3        |----->x+
    //   | /     | /        /
    //   |/______|/        /
    //  4       7         z+
    m_vertices = std::vector<Vertex>{
  /*
  * 2----1  y+
  * |    |  |
  * |    |  |
  * 3----0  .___ x-
  */
        Vertex{glm::vec3(-scale_x, -scale_y, -scale_z),
               glm::vec3(+0.0f, +0.0f, -1.0f),
               glm::vec3(-1.0f, +0.0f, +0.0f),
               glm::vec4(0.0f, 0.0f, 1.0f, 1.0f),
               glm::vec2(1.0f, 0.0f)}, // 0
        Vertex{glm::vec3(-scale_x, +scale_y, -scale_z),
               glm::vec3(+0.0f, +0.0f, -1.0f),
               glm::vec3(-1.0f, +0.0f, +0.0f),
               glm::vec4(0.0f, 0.0f, 1.0f, 1.0f),
               glm::vec2(1.0f, 1.0f)}, // 1
        Vertex{glm::vec3(+scale_x, +scale_y, -scale_z),
               glm::vec3(+0.0f, +0.0f, -1.0f),
               glm::vec3(-1.0f, +0.0f, +0.0f),
               glm::vec4(0.0f, 0.0f, 1.0f, 1.0f),
               glm::vec2(0.0f, 1.0f)}, // 2
        Vertex{glm::vec3(+scale_x, -scale_y, -scale_z),
               glm::vec3(+0.0f, +0.0f, -1.0f),
               glm::vec3(-1.0f, +0.0f, +0.0f),
               glm::vec4(0.0f, 0.0f, 1.0f, 1.0f),
               glm::vec2(0.0f, 0.0f)}, // 3

  /*
  * 6----2  y+
  * |    |  |
  * |    |  |
  * 7----3  .___ z-
  */
        Vertex{glm::vec3(+scale_x, +scale_y, -scale_z),
               glm::vec3(+1.0f, +0.0f, +0.0f),
               glm::vec3(+0.0f, +0.0f, -1.0f),
               glm::vec4(0.0f, 1.0f, 0.0f, 1.0f),
               glm::vec2(1.0f, 1.0f)}, // 4
        Vertex{glm::vec3(+scale_x, -scale_y, -scale_z),
               glm::vec3(+1.0f, +0.0f, +0.0f),
               glm::vec3(+0.0f, +0.0f, -1.0f),
               glm::vec4(0.0f, 1.0f, 0.0f, 1.0f),
               glm::vec2(1.0f, 0.0f)}, // 5
        Vertex{glm::vec3(+scale_x, +scale_y, +scale_z),
               glm::vec3(+1.0f, +0.0f, +0.0f),
               glm::vec3(+0.0f, +0.0f, -1.0f),
               glm::vec4(0.0f, 1.0f, 0.0f, 1.0f),
               glm::vec2(0.0f, 1.0f)}, // 6
        Vertex{glm::vec3(+scale_x, -scale_y, +scale_z),
               glm::vec3(+1.0f, +0.0f, +0.0f),
               glm::vec3(+0.0f, +0.0f, -1.0f),
               glm::vec4(0.0f, 1.0f, 0.0f, 1.0f),
               glm::vec2(0.0f, 0.0f)}, // 7

  /*
  * 5----6  y+
  * |    |  |
  * |    |  |
  * 4----7  .___ x+
  */
        Vertex{glm::vec3(-scale_x, -scale_y, +scale_z),
               glm::vec3(+0.0f, +0.0f, +1.0f),
               glm::vec3(+1.0f, +0.0f, +0.0f),
               glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),
               glm::vec2(0.0f, 0.0f)}, // 8
        Vertex{glm::vec3(-scale_x, +scale_y, +scale_z),
               glm::vec3(+0.0f, +0.0f, +1.0f),
               glm::vec3(+1.0f, +0.0f, +0.0f),
               glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),
               glm::vec2(0.0f, 1.0f)}, // 9
        Vertex{glm::vec3(+scale_x, +scale_y, +scale_z),
               glm::vec3(+0.0f, +0.0f, +1.0f),
               glm::vec3(+1.0f, +0.0f, +0.0f),
               glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),
               glm::vec2(1.0f, 1.0f)}, // 10
        Vertex{glm::vec3(+scale_x, -scale_y, +scale_z),
               glm::vec3(+0.0f, +0.0f, +1.0f),
               glm::vec3(+1.0f, +0.0f, +0.0f),
               glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),
               glm::vec2(1.0f, 0.0f)}, // 11

  // 0 1 4 5
  /*
  * 1----5  y+
  * |    |  |
  * |    |  |
  * 0----4  .___ z+
  */
        Vertex{glm::vec3(-scale_x, -scale_y, -scale_z),
               glm::vec3(-1.0f, +0.0f, +0.0f),
               glm::vec3(+0.0f, +0.0f, +1.0f),
               glm::vec4(1.0f, 1.0f, 0.0f, 1.0f),
               glm::vec2(0.0f, 0.0f)}, // 12
        Vertex{glm::vec3(-scale_x, +scale_y, -scale_z),
               glm::vec3(-1.0f, +0.0f, +0.0f),
               glm::vec3(+0.0f, +0.0f, +1.0f),
               glm::vec4(1.0f, 1.0f, 0.0f, 1.0f),
               glm::vec2(0.0f, 1.0f)}, // 13
        Vertex{glm::vec3(-scale_x, -scale_y, +scale_z),
               glm::vec3(-1.0f, +0.0f, +0.0f),
               glm::vec3(+0.0f, +0.0f, +1.0f),
               glm::vec4(1.0f, 1.0f, 0.0f, 1.0f),
               glm::vec2(1.0f, 0.0f)}, // 14
        Vertex{glm::vec3(-scale_x, +scale_y, +scale_z),
               glm::vec3(-1.0f, +0.0f, +0.0f),
               glm::vec3(+0.0f, +0.0f, +1.0f),
               glm::vec4(1.0f, 1.0f, 0.0f, 1.0f),
               glm::vec2(1.0f, 1.0f)}, // 15

  // 1 2 5 6
  /*
  * 1----2  z-
  * |    |  |
  * |    |  |
  * 5----6  .___ x+
  */
        Vertex{glm::vec3(-scale_x, +scale_y, -scale_z),
               glm::vec3(+0.0f, +1.0f, +0.0f),
               glm::vec3(+1.0f, +0.0f, +0.0f),
               glm::vec4(1.0f, 0.0f, 1.0f, 1.0f),
               glm::vec2(0.0f, 1.0f)}, // 16
        Vertex{glm::vec3(+scale_x, +scale_y, -scale_z),
               glm::vec3(+0.0f, +1.0f, +0.0f),
               glm::vec3(+1.0f, +0.0f, +0.0f),
               glm::vec4(1.0f, 0.0f, 1.0f, 1.0f),
               glm::vec2(1.0f, 1.0f)}, // 17
        Vertex{glm::vec3(-scale_x, +scale_y, +scale_z),
               glm::vec3(+0.0f, +1.0f, +0.0f),
               glm::vec3(+1.0f, +0.0f, +0.0f),
               glm::vec4(1.0f, 0.0f, 1.0f, 1.0f),
               glm::vec2(0.0f, 0.0f)}, // 18
        Vertex{glm::vec3(+scale_x, +scale_y, +scale_z),
               glm::vec3(+0.0f, +1.0f, +0.0f),
               glm::vec3(+1.0f, +0.0f, +0.0f),
               glm::vec4(1.0f, 0.0f, 1.0f, 1.0f),
               glm::vec2(1.0f, 0.0f)}, // 19

  // 0 3 4 7
  /*
  * 4----7  z+
  * |    |  |
  * |    |  |
  * 0----3  .___ x+
  */
        Vertex{glm::vec3(-scale_x, -scale_y, -scale_z),
               glm::vec3(+0.0f, -1.0f, +0.0f),
               glm::vec3(+1.0f, +0.0f, +0.0f),
               glm::vec4(0.0f, 1.0f, 1.0f, 1.0f),
               glm::vec2(0.0f, 0.0f)}, // 20
        Vertex{glm::vec3(+scale_x, -scale_y, -scale_z),
               glm::vec3(+0.0f, -1.0f, +0.0f),
               glm::vec3(+1.0f, +0.0f, +0.0f),
               glm::vec4(0.0f, 1.0f, 1.0f, 1.0f),
               glm::vec2(1.0f, 0.0f)}, // 21
        Vertex{glm::vec3(-scale_x, -scale_y, +scale_z),
               glm::vec3(+0.0f, -1.0f, +0.0f),
               glm::vec3(+1.0f, +0.0f, +0.0f),
               glm::vec4(0.0f, 1.0f, 1.0f, 1.0f),
               glm::vec2(0.0f, 1.0f)}, // 22
        Vertex{glm::vec3(+scale_x, -scale_y, +scale_z),
               glm::vec3(+0.0f, -1.0f, +0.0f),
               glm::vec3(+1.0f, +0.0f, +0.0f),
               glm::vec4(0.0f, 1.0f, 1.0f, 1.0f),
               glm::vec2(1.0f, 1.0f)}, // 23
    };

    m_indices =
        std::vector<size_t>{ 0,  1,  2,  0,  2,  3,  5,  4,  6,  5,  6,  7,
                             11, 10, 9,  11, 9,  8,  14, 15, 13, 14, 13, 12,
                             19, 17, 16, 19, 16, 18, 21, 23, 22, 21, 22, 20 };
}
