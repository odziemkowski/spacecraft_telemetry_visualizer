#pragma once

#include <cstdint>
#include <vector>

struct MeshData
{
    // Interleaved: position(3) + normal(3) + texcoord(2) = 8 floats per vertex
    std::vector<float> vertices;
    std::vector<uint32_t> indices;

    static constexpr int stride = 8 * sizeof(float);
    static constexpr int positionOffset = 0;
    static constexpr int normalOffset = 3 * sizeof(float);
    static constexpr int texCoordOffset = 6 * sizeof(float);
};

struct BoundingBox
{
    float min[3];
    float max[3];

    float maxExtent() const;
    void center(float out[3]) const;
};

MeshData generateUVSphere(float radius, int stacks, int sectors);
