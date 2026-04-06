#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "ObjLoader.h"

#include <limits>
#include <unordered_map>

namespace
{

struct IndexHash
{
    std::size_t operator()(const tinyobj::index_t &idx) const
    {
        auto h1 = std::hash<int>{}(idx.vertex_index);
        auto h2 = std::hash<int>{}(idx.normal_index);
        auto h3 = std::hash<int>{}(idx.texcoord_index);
        return h1 ^ (h2 << 16) ^ (h3 << 8);
    }
};

struct IndexEqual
{
    bool operator()(const tinyobj::index_t &a, const tinyobj::index_t &b) const
    {
        return a.vertex_index == b.vertex_index &&
               a.normal_index == b.normal_index &&
               a.texcoord_index == b.texcoord_index;
    }
};

} // namespace

ObjResult loadObj(const std::string &path)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    bool ok = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                               path.c_str(), nullptr, true);

    ObjResult result;

    if (!ok || shapes.empty())
        return result;

    constexpr float fmax = std::numeric_limits<float>::max();
    constexpr float fmin = std::numeric_limits<float>::lowest();
    result.bounds.min[0] = result.bounds.min[1] = result.bounds.min[2] = fmax;
    result.bounds.max[0] = result.bounds.max[1] = result.bounds.max[2] = fmin;

    std::unordered_map<tinyobj::index_t, uint32_t, IndexHash, IndexEqual> vertexMap;

    bool hasNormals = !attrib.normals.empty();
    bool hasTexCoords = !attrib.texcoords.empty();

    for (const auto &shape : shapes)
    {
        for (const auto &idx : shape.mesh.indices)
        {
            auto it = vertexMap.find(idx);
            if (it != vertexMap.end())
            {
                result.mesh.indices.push_back(it->second);
                continue;
            }

            uint32_t newIdx = static_cast<uint32_t>(
                result.mesh.vertices.size() / 8);
            vertexMap[idx] = newIdx;
            result.mesh.indices.push_back(newIdx);

            // Position
            float vx = attrib.vertices[3 * idx.vertex_index + 0];
            float vy = attrib.vertices[3 * idx.vertex_index + 1];
            float vz = attrib.vertices[3 * idx.vertex_index + 2];
            result.mesh.vertices.push_back(vx);
            result.mesh.vertices.push_back(vy);
            result.mesh.vertices.push_back(vz);

            // Update bounding box
            if (vx < result.bounds.min[0]) result.bounds.min[0] = vx;
            if (vy < result.bounds.min[1]) result.bounds.min[1] = vy;
            if (vz < result.bounds.min[2]) result.bounds.min[2] = vz;
            if (vx > result.bounds.max[0]) result.bounds.max[0] = vx;
            if (vy > result.bounds.max[1]) result.bounds.max[1] = vy;
            if (vz > result.bounds.max[2]) result.bounds.max[2] = vz;

            // Normal
            if (hasNormals && idx.normal_index >= 0)
            {
                result.mesh.vertices.push_back(attrib.normals[3 * idx.normal_index + 0]);
                result.mesh.vertices.push_back(attrib.normals[3 * idx.normal_index + 1]);
                result.mesh.vertices.push_back(attrib.normals[3 * idx.normal_index + 2]);
            }
            else
            {
                result.mesh.vertices.push_back(0.0f);
                result.mesh.vertices.push_back(1.0f);
                result.mesh.vertices.push_back(0.0f);
            }

            // TexCoord
            if (hasTexCoords && idx.texcoord_index >= 0)
            {
                result.mesh.vertices.push_back(attrib.texcoords[2 * idx.texcoord_index + 0]);
                result.mesh.vertices.push_back(attrib.texcoords[2 * idx.texcoord_index + 1]);
            }
            else
            {
                result.mesh.vertices.push_back(0.0f);
                result.mesh.vertices.push_back(0.0f);
            }
        }
    }

    return result;
}
