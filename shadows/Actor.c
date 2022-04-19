#include "Actor.h"
#include "Utils.h"

Actor* ActorNew(void)
{
	Actor* actor = malloc(sizeof(Actor));
	ActorInit(actor);
	return actor;
}

static void _ActorLoadMesh(Actor* actor, struct Mesh* mesh);
Actor* ActorFromMesh(struct Mesh* mesh)
{
	Actor* actor = ActorNew();
	_ActorLoadMesh(actor, mesh);
	return actor;
}

void ActorFree(Actor* actor)
{
	ActorDeinit(actor);
	free(actor);
}

void ActorInit(Actor* actor)
{
	memset(actor, 0, sizeof(Actor));
	actor->m_World = MathMat4X4Identity();
}

void ActorDeinit(Actor* actor)
{
	COM_FREE(actor->m_IndexBuffer);
	COM_FREE(actor->m_VertexBuffer);
	free(actor->m_Vertices);
	free(actor->m_Indices);
	actor->m_NumIndices = actor->m_NumVertices = 0;
	actor->m_Vertices = NULL;
	actor->m_Indices = NULL;
	actor->m_IndexBuffer = NULL;
	actor->m_VertexBuffer = NULL;
	for (size_t i = 0; i < ACTOR_NUM_TEXTURES; ++i)
	{
		if (actor->m_Textures[i])
		{
			COM_FREE(actor->m_Textures[i]);
		}
	}
}

static void _ActorLoadMesh(Actor* actor, struct Mesh* mesh)
{
	actor->m_Vertices = realloc(actor->m_Vertices, sizeof(Vertex) * (mesh->NumFaces + actor->m_NumVertices));
	actor->m_Indices = realloc(actor->m_Indices, sizeof(Vertex) * (mesh->NumFaces + actor->m_NumIndices));

	for (uint32_t j = 0; j < mesh->NumFaces; ++j)
	{
		const struct Face* face = mesh->Faces + j;
		const struct Position* pos = mesh->Positions + face->posIdx;
		const struct Normal* norm = mesh->Normals + face->normIdx;
		const struct TexCoord* tc = mesh->TexCoords + face->texIdx;
		Vertex* vert = actor->m_Vertices + actor->m_NumVertices;
		assert(face && pos && norm && tc && vert);

		vert->Position.X = pos->x;
		vert->Position.Y = pos->y;
		vert->Position.Z = pos->z;

		vert->Normal.X = norm->x;
		vert->Normal.Y = norm->y;
		vert->Normal.Z = norm->z;

		vert->TexCoords.X = tc->u;
		vert->TexCoords.Y = tc->v;

		assert(actor->m_NumIndices + 1 <= mesh->Faces);
		actor->m_Indices[actor->m_NumIndices] = actor->m_NumIndices++;

		actor->m_NumVertices++;
	}
}

static void _ActorLoadModel(Actor* actor, struct Model* model)
{
	size_t numFaces = 0;
	for (uint32_t i = 0; i < model->NumMeshes; ++i)
	{
		struct Mesh* mesh = model->Meshes + i;
		numFaces += mesh->NumFaces;
	}

	actor->m_Vertices = malloc(sizeof(Vertex) * numFaces);
	memset(actor->m_Vertices, 0, sizeof(Vertex) * numFaces);
	actor->m_Indices = malloc(sizeof(uint32_t) * numFaces);
	memset(actor->m_Indices, 0, sizeof(uint32_t) * numFaces);

	size_t posOffs = 0;
	size_t normOffs = 0;
	size_t tcOffs = 0;

	for (uint32_t i = 0; i < model->NumMeshes; ++i)
	{
		const struct Mesh* mesh = model->Meshes + i;
		for (uint32_t j = 0; j < mesh->NumFaces; ++j)
		{
			const struct Face* face = model->Meshes[i].Faces + j;
			const struct Position* pos = mesh->Positions + face->posIdx - posOffs;
			const struct Normal* norm = mesh->Normals + face->normIdx - normOffs;
			const struct TexCoord* tc = mesh->TexCoords + face->texIdx - tcOffs;
			Vertex* vert = actor->m_Vertices + actor->m_NumVertices;
			assert(face && pos && norm && tc && vert);

			vert->Position.X = pos->x;
			vert->Position.Y = pos->y;
			vert->Position.Z = pos->z;

			vert->Normal.X = norm->x;
			vert->Normal.Y = norm->y;
			vert->Normal.Z = norm->z;

			vert->TexCoords.X = tc->u;
			vert->TexCoords.Y = tc->v;

			assert(actor->m_NumIndices + 1 <= numFaces);
			actor->m_Indices[actor->m_NumIndices] = actor->m_NumIndices++;

			actor->m_NumVertices++;
		}
		posOffs += mesh->NumPositions;
		normOffs += mesh->NumNormals;
		tcOffs += mesh->NumTexCoords;
	}

	ModelFree(model);
}

void ActorLoadModel(Actor* actor, const char* filename)
{
	struct Model* model = OLLoad(filename);
	if (model)
	{
		_ActorLoadModel(actor, model);
	}
	else
	{
		UTILS_FATAL_ERROR("Failed to load model %s", filename);
	}
}

void ActorCreateVertexBuffer(Actor* actor, ID3D11Device* device)
{
	D3D11_SUBRESOURCE_DATA subresourceData = { 0 };
	subresourceData.pSysMem = actor->m_Vertices;

	D3D11_BUFFER_DESC bufferDesc = { 0 };
	bufferDesc.ByteWidth = sizeof(Vertex) * (uint32_t)actor->m_NumVertices;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.StructureByteStride = sizeof(Vertex);
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;

	HR(device->lpVtbl->CreateBuffer(device, &bufferDesc, &subresourceData,
		&actor->m_VertexBuffer))
}

void ActorCreateIndexBuffer(Actor* actor, ID3D11Device* device)
{
	D3D11_SUBRESOURCE_DATA subresourceData = { 0 };
	subresourceData.pSysMem = actor->m_Indices;

	D3D11_BUFFER_DESC bufferDesc = { 0 };
	bufferDesc.ByteWidth = sizeof(uint32_t) * actor->m_NumIndices;
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.StructureByteStride = 0;
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;

	HR(device->lpVtbl->CreateBuffer(device, &bufferDesc, &subresourceData, 
		&actor->m_IndexBuffer));
}

void ActorTranslate(Actor* actor, const Vec3D offset)
{
	const Mat4X4 offsetMat = MathMat4X4TranslateFromVec3D(&offset);
	actor->m_World = MathMat4X4MultMat4X4ByMat4X4(&actor->m_World,
		&offsetMat);
}

void ActorRotate(Actor* actor, const float pitch, const float yaw,
	const float roll)
{
	const Vec3D angles = { pitch, yaw, roll };
	const Mat4X4 rotMat = MathMat4X4RotateFromVec3D(&angles);
	actor->m_World = MathMat4X4MultMat4X4ByMat4X4(&actor->m_World,
		&rotMat);
}

void ActorScale(Actor* actor, const float s)
{
	const Vec3D scale = { s, s, s };
	const Mat4X4 scaleMat = MathMat4X4ScaleFromVec3D(&scale);
	actor->m_World = MathMat4X4MultMat4X4ByMat4X4(&actor->m_World,
		&scaleMat);
}