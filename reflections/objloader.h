#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

struct Position
{
	Position(): x{0}, y{0}, z{0} {}
	Position(float X, float Y, float Z): x{X}, y{Y}, z{Z} {}
	float x;
	float y;
	float z;
};

struct TexCoord
{
	TexCoord(): u{0}, v{0} {}
	TexCoord(float U, float V): u{U}, v{V} {}
	float u;
	float v;
};

struct Normal
{
	Normal() : x{0}, y{0}, z{0} {}
	Normal(float X, float Y, float Z): x{X}, y{Y}, z{Z} {}
	float x;
	float y;
	float z;
};

struct Face
{
	Face(): posIdx{0}, texIdx{0}, normIdx{0} {}
	Face(uint32_t pIdx, uint32_t tIdx, uint32_t nIdx) : posIdx{pIdx}, texIdx{tIdx}, normIdx{nIdx} {}
	uint32_t posIdx;
	uint32_t texIdx;
	uint32_t normIdx;
};

struct Mesh
{
	std::string Name;
	std::vector<Position> Positions;
	std::vector<TexCoord> TexCoords;
	std::vector<Normal> Normals;
	std::vector<Face> Faces;
};

struct Model
{
	std::vector<Mesh> Meshes;
	std::string Directory;
};

std::unique_ptr<Model> OLLoad(const char* filename);
void OLDumpModelToFile(const Model* model, const char* filename);

