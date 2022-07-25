#pragma once

#include <cstdlib>
#include <cstring>
#include <memory>
#include <vector>

#include <Windows.h>

#include "NE_Math.h"
#include "objloader.h"

std::unique_ptr<Mesh> MGGeneratePlane (const Vec3D *origin, const float width,
                                       const float height);
std::unique_ptr<Mesh> MGCreateSphere (float radius, unsigned int rings,
                                      unsigned int sectors);