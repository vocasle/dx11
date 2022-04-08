#pragma once

#include <d3d11.h>

#include "Math.h"

struct Texture
{
	Texture();
	~Texture();
	ID3D11Texture2D* Resource;
	ID3D11ShaderResourceView* SRV;
};

struct Material
{
	Vec4D Ambient;
	Vec4D Diffuse;
	Vec4D Specular;
};

struct PointLight
{
	Vec4D Ambient;
	Vec4D Diffuse;
	Vec4D Specular;
	Vec3D Position;
	float _Pad;
};

