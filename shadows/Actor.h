#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <vector>

#include "Math.h"
#include "objloader.h"
#include "LightHelper.h"

#define ACTOR_NUM_TEXTURES 4

enum class TextureType
{
	Diffuse = 0,
	Specular = 1,
	Gloss = 2,
	Normal = 3,
};

struct Vertex
{
	Vec3D Position;
	Vec3D Normal;
	Vec2D TexCoords;
};

class Actor
{
public:
	Actor();
	Actor(const Actor& actor);
	Actor& operator=(const Actor& actor);
	Actor(Actor&& actor) noexcept;
	Actor& operator=(Actor&& actor) noexcept;
	Actor(Mesh* mesh);
	~Actor();

	void LoadModel(const char* filename);

	void CreateVertexBuffer(ID3D11Device* device);
	void CreateIndexBuffer(ID3D11Device* device);

	void Translate(const Vec3D offset);
	void Rotate(const float pitch, const float yaw, const float roll);
	void Scale(const float s);

	void LoadTexture(const char* filename,
		enum TextureType type,
		ID3D11Device* device,
		ID3D11DeviceContext* context);

	void SetMaterial(const Material* material);

private:
	void Swap(Actor& actor);
	void LoadMesh(Mesh* mesh);

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_IndexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_VertexBuffer;
	std::vector<Vertex> m_Vertices;
	std::vector<uint32_t> m_Indices;
	Mat4X4 m_World;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_DiffuseTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_SpecularTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_GlossTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_NormalTexture;
	Material m_Material;
};