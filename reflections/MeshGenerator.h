#pragma once

#include <cstdlib>
#include <cstring>
#include <vector>
#include <memory>

#include <Windows.h>

#include "Math.h"
#include "objloader.h"

std::unique_ptr<Mesh> MGGeneratePlane(const Vec3D* origin, const float width, const float height);

struct Vertex;

HRESULT ComputeTangentFrame(const std::vector<uint32_t>& indices, std::vector<Vertex>& vertices);