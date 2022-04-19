#pragma once

#include <d3d11.h>

#include "Math.h"
#include "objloader.h"

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
	Mat4X4 m_World;
} Actor;

Actor* ActorNew(void);
Actor* ActorFromMesh(struct Mesh* mesh);
void ActorFree(Actor* actor);

void ActorInit(Actor* actor);
void ActorDeinit(Actor* actor);

void ActorLoadModel(Actor* actor, const char* filename);

void ActorCreateVertexBuffer(Actor* actor, ID3D11Device* device);
void ActorCreateIndexBuffer(Actor* actor, ID3D11Device* device);

void ActorTranslate(Actor* actor, const Vec3D offset);
void ActorRotate(Actor* actor, const float pitch, const float yaw, 
	const float roll);
void ActorScale(Actor* actor, const float s);