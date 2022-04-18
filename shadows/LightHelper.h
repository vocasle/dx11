#pragma once

#include "Math.h"

struct Color
{
	float R;
	float G;
	float B;
	float A;
} typedef Color;

inline Color ColorFromRGBA(const float r, const float g, const float b, const float a)
{
	Color color = { r, g, b, a };
	return color;
}

typedef struct DirectionalLight
{
	Color Ambient;
	Color Diffuse;
	Color Specular;
	Vec3D Direction;
	float pad;
} DirectionalLight;

typedef struct PointLight
{
	Color Ambient;
	Color Diffuse;
	Color Specular;
	Vec3D Position;
	float Range;
	Vec3D Att;
	float pad;
} PointLight;

typedef struct SpotLight
{
	Color Ambient;
	Color Diffuse;
	Color Specular;
	Vec3D Position;
	float Range;
	Vec3D Direction;
	float Spot;
	Vec3D Att;
	float pad;
} SpotLight;

typedef struct Material
{
	Vec4D Ambient;
	Vec4D Diffuse;
	Vec4D Specular;
} Material;
