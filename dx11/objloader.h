#pragma once

#include <stdint.h>

struct Position
{
	Position(): x(0.0f), y(0.0f), z(0.0f) {}
	float x;
	float y;
	float z;
};

struct TexCoord
{
	TexCoord(): u(0.0f), v(0.0f) {}
	float u;
	float v;
};

struct Normal
{
	Normal(): x(0.0f), y(0.0f), z(0.0f) {}
	float x;
	float y;
	float z;
};

struct Face
{
	Face(): posIdx(0), texIdx(0), normIdx(0) {}
	uint32_t posIdx;
	uint32_t texIdx;
	uint32_t normIdx;
};

struct Mesh
{
	Mesh();
	char* Name;
	struct Position* Positions;
	uint32_t NumPositions;
	struct TexCoord* TexCoords;
	uint32_t NumTexCoords;
	struct Normal* Normals;
	uint32_t NumNormals;
	struct Face* Faces;
	uint32_t NumFaces;
};

struct Model
{
	Model(): Meshes(nullptr), NumMeshes(0), Directory(nullptr) {}
	struct Mesh* Meshes;
	uint32_t NumMeshes;
	char* Directory;
};

struct Model* OLLoad(const char* filename);
void OLDumpModelToFile(const struct Model* model, const char* filename);

struct Mesh* MeshNew(void);
void MeshFree(struct Mesh* mesh);
void MeshDeinit(struct Mesh* mesh);

struct Model* ModelNew(void);
void ModelFree(struct Model* model);
void ModelDeinit(struct Model* model);

