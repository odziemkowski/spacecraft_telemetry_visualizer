#pragma once

#include "Geometry.h"

#include <string>

struct ObjResult
{
    MeshData mesh;
    BoundingBox bounds;
};

ObjResult loadObj(const std::string &path);
