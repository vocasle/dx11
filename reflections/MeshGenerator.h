#pragma once

#include <cstdlib>
#include <cstring>
#include <vector>
#include <memory>

#include <Windows.h>

#include "Math.h"
#include "objloader.h"

inline struct std::unique_ptr<Mesh> MGGeneratePlane(const Vec3D* origin, const float width, const float height)
{
	std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>();
	
	mesh->Positions.reserve(4);
	mesh->Positions.emplace_back(origin->X + width / 2.0f, origin->Y, origin->Z - height / 2.0f);
	mesh->Positions.emplace_back(origin->X - width / 2.0f, origin->Y, origin->Z - height / 2.0f);
	mesh->Positions.emplace_back(origin->X - width / 2.0f, origin->Y, origin->Z + height / 2.0f);
	mesh->Positions.emplace_back(origin->X + width / 2.0f, origin->Y, origin->Z + height / 2.0f);

	mesh->Normals.emplace_back(0.0f, -1.0f, 0.0f);

	mesh->TexCoords.reserve(4);
	mesh->TexCoords.emplace_back(1.0f, 1.0f);
	mesh->TexCoords.emplace_back(0.0f, 1.0f);
	mesh->TexCoords.emplace_back(0.0f, 0.0f);
	mesh->TexCoords.emplace_back(1.0f, 0.0f);

	mesh->Faces = std::vector<Face>(6);
	mesh->Faces[0].normIdx = 0;
	mesh->Faces[0].posIdx = 0;
	mesh->Faces[0].texIdx = 0;

	mesh->Faces[1].normIdx = 0;
	mesh->Faces[1].posIdx = 1;
	mesh->Faces[1].texIdx = 1;

	mesh->Faces[2].normIdx = 0;
	mesh->Faces[2].posIdx = 2;
	mesh->Faces[2].texIdx = 2;

	mesh->Faces[3].normIdx = 0;
	mesh->Faces[3].posIdx = 2;
	mesh->Faces[3].texIdx = 2;

	mesh->Faces[4].normIdx = 0;
	mesh->Faces[4].posIdx = 3;
	mesh->Faces[4].texIdx = 3;

	mesh->Faces[5].normIdx = 0;
	mesh->Faces[5].posIdx = 0;
	mesh->Faces[5].texIdx = 0;

	return mesh;
}

struct Vertex;

HRESULT ComputeTangentFrame(const std::vector<uint32_t>& indices, std::vector<Vertex>& vertices);