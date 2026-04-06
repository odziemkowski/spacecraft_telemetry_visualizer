#include "Geometry.h"

#include <algorithm>
#include <cmath>

float BoundingBox::maxExtent() const
{
    return std::max({max[0] - min[0], max[1] - min[1], max[2] - min[2]});
}

void BoundingBox::center(float out[3]) const
{
    out[0] = (min[0] + max[0]) / 2.0f;
    out[1] = (min[1] + max[1]) / 2.0f;
    out[2] = (min[2] + max[2]) / 2.0f;
}

MeshData generateUVSphere(float radius, int stacks, int sectors)
{
    MeshData mesh;

    // fixme pi definition is duplicated in multiple places; consider centralizing it
    const float pi = 3.14159265358979323846f;

    // Generate vertices
    for (int i = 0; i <= stacks; ++i)
    {
        float phi = pi * float(i) / float(stacks); // 0 to pi (top to bottom)
        float sinPhi = std::sin(phi);
        float cosPhi = std::cos(phi);

        for (int j = 0; j <= sectors; ++j)
        {
            float theta = 2.0f * pi * float(j) / float(sectors); // 0 to 2pi
            float sinTheta = std::sin(theta);
            float cosTheta = std::cos(theta);

            // Position
            float x = cosTheta * sinPhi;
            float y = cosPhi;
            float z = sinTheta * sinPhi;

            mesh.vertices.push_back(radius * x); // position
            mesh.vertices.push_back(radius * y);
            mesh.vertices.push_back(radius * z);

            mesh.vertices.push_back(x); // normal (unit sphere)
            mesh.vertices.push_back(y);
            mesh.vertices.push_back(z);

            float u = float(j) / float(sectors);
            float v = float(i) / float(stacks);
            mesh.vertices.push_back(u); // texcoord
            mesh.vertices.push_back(v);
        }
    }

    // Generate indices (two triangles per quad)
    for (int i = 0; i < stacks; ++i)
    {
        for (int j = 0; j < sectors; ++j)
        {
            uint32_t topLeft = i * (sectors + 1) + j;
            uint32_t bottomLeft = (i + 1) * (sectors + 1) + j;
            uint32_t topRight = topLeft + 1;
            uint32_t bottomRight = bottomLeft + 1;

            // Skip degenerate triangles at poles
            if (i != 0)
            {
                mesh.indices.push_back(topLeft);
                mesh.indices.push_back(bottomLeft);
                mesh.indices.push_back(topRight);
            }
            if (i != stacks - 1)
            {
                mesh.indices.push_back(topRight);
                mesh.indices.push_back(bottomLeft);
                mesh.indices.push_back(bottomRight);
            }
        }
    }

    return mesh;
}
