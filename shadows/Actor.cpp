#include "Actor.h"
#include "Utils.h"
#include "stb_image.h"

Actor::Actor():
	m_IndexBuffer{nullptr},
	m_VertexBuffer{nullptr},
	m_Vertices{},
	m_Indices{},
	m_World{MathMat4X4Identity()},
	m_Textures{},
	m_Material{}
{
}

Actor::Actor(Mesh* mesh): Actor()
{
	LoadMesh(mesh);
}

Actor::~Actor()
{
	for (uint32_t i = 0; i < ACTOR_NUM_TEXTURES; ++i)
	{
		if (m_Textures[i])
		{
			COM_FREE(m_Textures[i]);
		}
	}
}

void Actor::LoadMesh(Mesh* mesh)
{
	m_Vertices.reserve(mesh->NumFaces + m_Vertices.size());
	m_Indices.reserve(mesh->NumFaces + m_Indices.size());

	Vertex vert = {};

	for (uint32_t j = 0; j < mesh->NumFaces; ++j)
	{
		const struct Face* face = mesh->Faces + j;
		const struct Position* pos = mesh->Positions + face->posIdx;
		const struct Normal* norm = mesh->Normals + face->normIdx;
		const struct TexCoord* tc = mesh->TexCoords + face->texIdx;
		assert(face && pos && norm && tc);

		vert.Position.X = pos->x;
		vert.Position.Y = pos->y;
		vert.Position.Z = pos->z;

		vert.Normal.X = norm->x;
		vert.Normal.Y = norm->y;
		vert.Normal.Z = norm->z;

		vert.TexCoords.X = tc->u;
		vert.TexCoords.Y = tc->v;

		assert(m_Indices.size() + 1 <= mesh->NumFaces);
		m_Indices.emplace_back(m_Indices.size());
		assert(m_Vertices.size() + 1 <= mesh->NumFaces);
		m_Vertices.emplace_back(vert);
	}
}

void Actor::LoadModel(const char* filename)
{
	struct Model* model = OLLoad(filename);
	if (!model)
	{
		UTILS_FATAL_ERROR("Failed to load model %s", filename);
	}

	size_t numFaces = 0;
	for (uint32_t i = 0; i < model->NumMeshes; ++i)
	{
		struct Mesh* mesh = model->Meshes + i;
		numFaces += mesh->NumFaces;
	}

	m_Vertices.reserve(numFaces);
	m_Indices.reserve(numFaces);

	size_t posOffs = 0;
	size_t normOffs = 0;
	size_t tcOffs = 0;
	Vertex vert = {};

	for (uint32_t i = 0; i < model->NumMeshes; ++i)
	{
		const struct Mesh* mesh = model->Meshes + i;
		for (uint32_t j = 0; j < mesh->NumFaces; ++j)
		{
			const struct Face* face = model->Meshes[i].Faces + j;
			const struct Position* pos = mesh->Positions + face->posIdx - posOffs;
			const struct Normal* norm = mesh->Normals + face->normIdx - normOffs;
			const struct TexCoord* tc = mesh->TexCoords + face->texIdx - tcOffs;
			assert(face && pos && norm && tc);

			vert.Position.X = pos->x;
			vert.Position.Y = pos->y;
			vert.Position.Z = pos->z;

			vert.Normal.X = norm->x;
			vert.Normal.Y = norm->y;
			vert.Normal.Z = norm->z;

			vert.TexCoords.X = tc->u;
			vert.TexCoords.Y = tc->v;

			assert(m_Indices.size() + 1 <= mesh->NumFaces);
			m_Indices.emplace_back(m_Indices.size());
			assert(m_Vertices.size() + 1 <= mesh->NumFaces);
			m_Vertices.emplace_back(vert);
		}
		posOffs += mesh->NumPositions;
		normOffs += mesh->NumNormals;
		tcOffs += mesh->NumTexCoords;
	}

	ModelFree(model);
}

void Actor::CreateVertexBuffer(ID3D11Device* device)
{
	D3D11_SUBRESOURCE_DATA subresourceData = {};
	subresourceData.pSysMem = &m_Vertices[0];

	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.ByteWidth = sizeof(Vertex) * m_Vertices.size();
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.StructureByteStride = sizeof(Vertex);
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;

	HR(device->CreateBuffer( &bufferDesc, &subresourceData,
		&m_VertexBuffer))
}

void Actor::CreateIndexBuffer(ID3D11Device* device)
{
	D3D11_SUBRESOURCE_DATA subresourceData = {};
	subresourceData.pSysMem = &m_Indices[0];

	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.ByteWidth = sizeof(uint32_t) * m_Indices.size();
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.StructureByteStride = 0;
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;

	HR(device->CreateBuffer( &bufferDesc, &subresourceData, 
		&m_IndexBuffer));
}

void Actor::Translate(const Vec3D offset)
{
	const Mat4X4 offsetMat = MathMat4X4TranslateFromVec3D(&offset);
	m_World = MathMat4X4MultMat4X4ByMat4X4(&m_World,
		&offsetMat);
}

void Actor::Rotate(const float pitch, const float yaw, const float roll)
{
	const Vec3D angles = { pitch, yaw, roll };
	const Mat4X4 rotMat = MathMat4X4RotateFromVec3D(&angles);
	m_World = MathMat4X4MultMat4X4ByMat4X4(&m_World,
		&rotMat);
}

void Actor::Scale(const float s)
{
	const Vec3D scale = { s, s, s };
	const Mat4X4 scaleMat = MathMat4X4ScaleFromVec3D(&scale);
	m_World = MathMat4X4MultMat4X4ByMat4X4(&m_World, &scaleMat);
}

void Actor::LoadTexture(const char* filename,
	enum TextureType type,
	ID3D11Device* device,
	ID3D11DeviceContext* context)
{
	int width = 0;
	int height = 0;
	int channelsInFile = 0;
	const int desiredChannels = 4;

	unsigned char* bytes = stbi_load(filename, &width, &height, &channelsInFile, desiredChannels);
	if (!bytes)
	{
		UtilsDebugPrint("ERROR: Failed to load texture from %s\n", filename);
		ExitProcess(EXIT_FAILURE);
	}
	ID3D11Texture2D* texture = NULL;
	{
		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

		D3D11_SUBRESOURCE_DATA subresourceData = {};
		subresourceData.pSysMem = bytes;
		subresourceData.SysMemPitch = width * sizeof(unsigned char) * desiredChannels;

		HR(device->CreateTexture2D( &desc, &subresourceData, &texture))
	}

	{
		// TODO: Fix mip map generation
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		memset(&srvDesc, 0, sizeof(srvDesc));
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = -1;
		HR(device->CreateShaderResourceView( (ID3D11Resource*)texture, &srvDesc, &m_Textures[static_cast<uint32_t>(type)]))

		context->GenerateMips(m_Textures[static_cast<uint32_t>(type)]);
	}
	COM_FREE(texture);

	stbi_image_free(bytes);
}

void Actor::SetMaterial(const Material* material)
{
	m_Material = *material;
}