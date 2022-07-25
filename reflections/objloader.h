#pragma once

#include "Math.h"
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

struct Face
{
  Face () : posIdx{ 0 }, texIdx{ 0 }, normIdx{ 0 } {}
  Face (uint32_t pIdx, uint32_t tIdx, uint32_t nIdx)
      : posIdx{ pIdx }, texIdx{ tIdx }, normIdx{ nIdx }
  {
  }
  uint32_t posIdx;
  uint32_t texIdx;
  uint32_t normIdx;
};

struct Mesh
{
  std::string Name;
  std::vector<Vec3D> Positions;
  std::vector<Vec2D> TexCoords;
  std::vector<Vec3D> Normals;
  std::vector<Face> Faces;
};

struct Model
{
  std::vector<Mesh> Meshes;
  std::string Directory;
};

std::unique_ptr<Model> OLLoad (const char *filename);
void OLDumpModelToFile (const Model *model, const char *filename);
