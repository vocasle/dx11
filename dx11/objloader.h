#pragma once

#include <stdint.h>
#include <string>
#include <vector>

struct Position
{
	Position(): x(0.0f), y(0.0f), z(0.0f) {}
	Position(float inX, float inY, float inZ) : x(inX), y(inY), z(inZ) {}
	float x;
	float y;
	float z;
};

struct TexCoord
{
	TexCoord(): u(0.0f), v(0.0f) {}
	TexCoord(float inU, float inV) : u(inU), v(inV) {}
	float u;
	float v;
};

struct Normal
{
	Normal(): x(0.0f), y(0.0f), z(0.0f) {}
	Normal(float inX, float inY, float inZ) : x(inX), y(inY), z(inZ) {}
	float x;
	float y;
	float z;
};

struct Face
{
	Face(): posIdx(0), texIdx(0), normIdx(0) {}
	Face(uint32_t inPosIdx, uint32_t inTexIdx, uint32_t inNormIdx) 
		: posIdx(inPosIdx), texIdx(inTexIdx), normIdx(inNormIdx) {}
	uint32_t posIdx;
	uint32_t texIdx;
	uint32_t normIdx;
};

struct Mesh
{
	Mesh();
	std::string Name;
	std::vector<Position> Positions;
	std::vector<TexCoord> TexCoords;
	std::vector<Normal> Normals;
	std::vector<Face> Faces;
};

struct Model
{
	Model(): Meshes(), Directory() {}
	std::vector<Mesh> Meshes;
	std::string Directory;
};

struct Model* OLLoad(const char* filename);
void OLDumpModelToFile(const struct Model* model, const char* filename);

