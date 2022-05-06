#include "MeshGenerator.h"
#include "Actor.h"

#include <cassert>

std::unique_ptr<Mesh> MGGeneratePlane(const Vec3D* origin, const float width, const float height)
{
    std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>();

    mesh->Positions.reserve(4);
    mesh->Positions.emplace_back(origin->X + width / 2.0f, origin->Y, origin->Z - height / 2.0f);
    mesh->Positions.emplace_back(origin->X - width / 2.0f, origin->Y, origin->Z - height / 2.0f);
    mesh->Positions.emplace_back(origin->X - width / 2.0f, origin->Y, origin->Z + height / 2.0f);
    mesh->Positions.emplace_back(origin->X + width / 2.0f, origin->Y, origin->Z + height / 2.0f);

    mesh->Normals.emplace_back(0.0f, 1.0f, 0.0f);

    mesh->TexCoords.reserve(4);
    mesh->TexCoords.emplace_back(1.0f, 1.0f);
    mesh->TexCoords.emplace_back(0.0f, 1.0f);
    mesh->TexCoords.emplace_back(0.0f, 0.0f);
    mesh->TexCoords.emplace_back(1.0f, 0.0f);

    mesh->Faces.reserve(6);
    mesh->Faces.emplace_back(0, 0, 0);
    mesh->Faces.emplace_back(1, 1, 0);
    mesh->Faces.emplace_back(2, 2, 0);
    mesh->Faces.emplace_back(2, 2, 0);
    mesh->Faces.emplace_back(3, 3, 0);
    mesh->Faces.emplace_back(0, 0, 0);

    return mesh;
}