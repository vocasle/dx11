#pragma once

#include <d3d11.h>

#include "Math.h"

typedef struct _Vertex
{
	Vec3D Position;
	Vec3D Normal;
	Vec2D TexCoords;
} Vertex;

typedef struct _Actor
{
	ID3D11Buffer* m_IndexBuffer;
	ID3D11Buffer* m_VertexBuffer;
	Vertex* m_Vertices;
	size_t m_NumVertices;
	uint32_t* m_Indices;
	size_t m_NumIndices;
} Actor;

Actor* ActorNew(void);
void ActorFree(Actor* actor);

void ActorInit(Actor* actor);
void ActorDeinit(Actor* actor);

void ActorLoadModel(Actor* actor, const char* filename);

void ActorCreateVertexBuffer(Actor* actor, ID3D11Device* device);
void ActorCreateIndexBuffer(Actor* actor, ID3D11Device* device);