#pragma once

#include <stdlib.h>
#include <string.h>

#include "Math.h"
#include "objloader.h"


inline struct Mesh* MGGeneratePlane(const Vec3D* origin, const float width, const float height)
{
	struct Mesh* mesh = MeshNew();
	
	mesh->Positions = new Position[4];
	mesh->Positions[0].x = origin->X + width / 2.0f;
	mesh->Positions[0].y = origin->Y;
	mesh->Positions[0].z = origin->Z - height / 2.0f;

	mesh->Positions[1].x = origin->X - width / 2.0f;
	mesh->Positions[1].y = origin->Y;
	mesh->Positions[1].z = origin->Z - height / 2.0f;

	mesh->Positions[2].x = origin->X - width / 2.0f;
	mesh->Positions[2].y = origin->Y;
	mesh->Positions[2].z = origin->Z + height / 2.0f;

	mesh->Positions[3].x = origin->X + width / 2.0f;
	mesh->Positions[3].y = origin->Y;
	mesh->Positions[3].z = origin->Z + height / 2.0f;
	mesh->NumPositions = 4;

	mesh->Normals = new Normal;
	mesh->Normals[0].x = 0.0f;
	mesh->Normals[0].y = -1.0f;
	mesh->Normals[0].z = 0.0f;
	mesh->NumNormals = 1;

	mesh->TexCoords = new TexCoord[4];
	mesh->TexCoords[0].u = 1.0f;
	mesh->TexCoords[0].v = 1.0f;

	mesh->TexCoords[1].u = 0.0f;
	mesh->TexCoords[1].v = 1.0f;

	mesh->TexCoords[2].u = 0.0f;
	mesh->TexCoords[2].v = 0.0f;

	mesh->TexCoords[3].u = 1.0f;
	mesh->TexCoords[3].v = 0.0f;
	mesh->NumTexCoords = 4;

	mesh->Faces = new Face[6];
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
	mesh->NumFaces = 6;

	return mesh;
}
