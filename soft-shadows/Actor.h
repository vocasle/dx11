#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <vector>

#include "NE_Math.h"
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
	Vec3D Tangent;
	Vec3D Bitangent;
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
	void Scale(const float x, const float y, const float z);

	void LoadTexture(const char* filename,
		enum TextureType type,
		ID3D11Device* device,
		ID3D11DeviceContext* context);

	void SetMaterial(const Material* material);
	void SetTexture(ID3D11ShaderResourceView* srv, TextureType type);
	void SetTextures(ID3D11ShaderResourceView* srv[4]);

	Mat4X4 GetWorld() const;
	Material GetMaterial() const { return m_Material; }
	ID3D11Buffer* GetIndexBuffer() const { return m_IndexBuffer.Get(); }
	ID3D11Buffer* GetVertexBuffer() const { return m_VertexBuffer.Get(); }
	uint32_t GetNumIndices() const { return m_Indices.size(); }
	ID3D11ShaderResourceView** GetShaderResources() const;
	bool IsVisible() const { return m_IsVisible; }
	void SetIsVisible(const bool isVisible);
	const std::string& GetName() const { return m_Name; };
	void SetName(const std::string& name);
	const std::vector<Vertex>& GetVertices() const { return m_Vertices; }
	const std::vector<uint32_t>& GetIndices() const { return m_Indices; }

private:
	void Swap(Actor& actor);
	void LoadMesh(Mesh* mesh);
	void GenerateTangents();

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_IndexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_VertexBuffer;
	std::vector<Vertex> m_Vertices;
	std::vector<uint32_t> m_Indices;
	Mat4X4 m_Rotation;
	Mat4X4 m_Scale;
	Mat4X4 m_Translation;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_DiffuseTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_SpecularTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_GlossTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_NormalTexture;
	Material m_Material;
	std::string m_Name;
	bool m_IsVisible;
};
