#include "Actor.h"
#include "Utils.h"
#include "objloader.h"

Actor* ActorNew(void)
{
	Actor* actor = malloc(sizeof(Actor));
	ActorInit(actor);
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