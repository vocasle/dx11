#include "ParticleSystem.h"
#include "Image.h"
#include "InputLayout.h"

#include <cassert>

using namespace Microsoft::WRL;


ParticleSystem::ParticleSystem(const std::string& name, const Vec3D& origin, const Camera& camera)
	: m_name{name},
	m_origin{origin},
	m_camera{&camera},
	m_emitter{ ParticleType::Emitter, {}, {}, {}, 0.0f }
{
}

ParticleSystem::~ParticleSystem()
{
}

void ParticleSystem::Init(ID3D11Device* device, ID3D11DeviceContext* context, const std::string& texFilePath)
{
	CreateTexture(device, context, texFilePath);
	CreateBlendState(device);
	CreateDepthStencilState(device);
	CreateSamplerState(device);
	CreateEmitter();
	CreateVertexBuffer(device);
	CreateIndexBuffer(device);
}

void ParticleSystem::Tick(const float deltaTime)
{
	//for (Emitter& e : m_emitters)
	//{
	//	e.Tick(deltaTime);
	//}

	//for (Particle& p : m_particles)
	//{
	//	p.Tick(deltaTime);
	//}

	//UpdateVertices();
}

void ParticleSystem::CreateTexture(ID3D11Device* device, ID3D11DeviceContext* context, const std::string& filepath)
{
	D3D11_TEXTURE2D_DESC desc = {};
	Image image{ filepath };

	ComPtr<ID3D11Texture2D> texture;
	{
		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = image.GetWidth();
		desc.Height = image.GetHeight();
		desc.MipLevels = 0;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

		HR(device->CreateTexture2D(&desc, nullptr, texture.ReleaseAndGetAddressOf()))

		context->UpdateSubresource(texture.Get(), 0, nullptr, image.GetBytes(), 
			image.GetWidth() * sizeof(uint8_t) * Image::DESIRED_CHANNELS, 0);
	}

	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = -1;

		HR(device->CreateShaderResourceView(texture.Get(), &srvDesc, m_diffuseTexture.ReleaseAndGetAddressOf()))
		context->GenerateMips(m_diffuseTexture.Get());
	}
}

void ParticleSystem::CreateBlendState(ID3D11Device* device)
{
	CD3D11_BLEND_DESC desc = CD3D11_BLEND_DESC{ CD3D11_DEFAULT{} };
	desc.RenderTarget[0].BlendEnable = true;
	desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	desc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;

	HR(device->CreateBlendState(&desc, m_blendState.ReleaseAndGetAddressOf()))
}

void ParticleSystem::CreateDepthStencilState(ID3D11Device* device)
{
	CD3D11_DEPTH_STENCIL_DESC desc = CD3D11_DEPTH_STENCIL_DESC{ CD3D11_DEFAULT{} };

	HR(device->CreateDepthStencilState(&desc, m_depthStencilState.ReleaseAndGetAddressOf()))
}

void ParticleSystem::CreateVertexBuffer(ID3D11Device* device)
{
	D3D11_SUBRESOURCE_DATA subresourceData = {};
	subresourceData.pSysMem = &m_vertices[0];

	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.ByteWidth = sizeof(Particle::Vertex) * m_vertices.size();
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.StructureByteStride = sizeof(Particle::Vertex);
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;

	HR(device->CreateBuffer(&bufferDesc, &subresourceData, m_vertexBuffer.ReleaseAndGetAddressOf()))
}

void ParticleSystem::CreateIndexBuffer(ID3D11Device* device)
{
	D3D11_SUBRESOURCE_DATA subresourceData = {};
	subresourceData.pSysMem = &m_indices[0];

	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.ByteWidth = sizeof(uint32_t) * m_indices.size();
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.StructureByteStride = 0;
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;

	HR(device->CreateBuffer(&bufferDesc, &subresourceData, m_indexBuffer.ReleaseAndGetAddressOf()))
}

void ParticleSystem::CreateSamplerState(ID3D11Device* device)
{
	CD3D11_SAMPLER_DESC desc = CD3D11_SAMPLER_DESC{ CD3D11_DEFAULT{} };
	HR(device->CreateSamplerState(&desc, m_sampler.ReleaseAndGetAddressOf()))
}

void ParticleSystem::UpdateVertices()
{
	//m_vertices.clear();
	//m_indices.clear();
	//size_t indexOffset = 0;
	//for (const Particle& p : m_particles)
	//{
	//	for (int i = 0; i < 4; ++i)
	//	{
	//		m_vertices.push_back(p.m_vertices[i]);
	//	}

	//	for (int i = 0; i < 6; ++i)
	//	{
	//		m_indices.push_back(p.m_indices[i] + indexOffset);
	//	}
	//	indexOffset += 6;
	//}
}

void ParticleSystem::CreateEmitter()
{
	const Vec3D accel = { 0.0f, -9.8f, 0.0f };
	const Vec3D initVel = {};
	const Vec3D origin = {};
	Particle p = {ParticleType::Emitter, accel, initVel, origin, 0.0f};
	m_emitter = p;
	EmitParticle();


}

void ParticleSystem::EmitParticle()
{
	Particle p = { ParticleType::Particle, m_emitter.GetAccel(), m_emitter.GetInitVel(), m_emitter.GetInitPos(), MAX_LIFESPAN };
	p.CreateQuad(5, 5, m_camera->GetUp(), m_camera->GetRight());
	m_particles.push_back(p);
	m_vertices.insert(m_vertices.end(), p.GetVertices().begin(), p.GetVertices().end());
	m_indices.insert(m_indices.end(), p.GetIndices().begin(), p.GetIndices().end());
}

//void ParticleSystem::Particle::CreateQuad()
//{
//	Vec3D R = m_camera->GetRight();
//	MathVec3DNormalize(&R);
//	Vec3D U = m_camera->GetUp();
//	MathVec3DNormalize(&U);
//	const Vec3D P = m_pos;
//
//	Vec3D X = (m_width / 2.0f) * R;
//	Vec3D Y = (m_width / 2.0f) * U;
//	const Vec3D Q1 = P + X + Y;
//	const Vec3D Q4 = P - X + Y; // q4
//	const Vec3D Q3 = P - X - Y; // q3
//	const Vec3D Q2 = P + X - Y; // q2
//
//	m_vertices[0].Position = Q1;
//	m_vertices[1].Position = Q2;
//	m_vertices[2].Position = Q3;
//	m_vertices[3].Position = Q4;
//	m_vertices[0].Lifespan = m_lifespan;
//	m_vertices[1].Lifespan = m_lifespan;
//	m_vertices[2].Lifespan = m_lifespan;
//	m_vertices[3].Lifespan = m_lifespan;
//
//	m_indices[0] = 0;
//	m_indices[1] = 1;
//	m_indices[2] = 2;
//	m_indices[3] = 2;
//	m_indices[4] = 3;
//	m_indices[5] = 0;
//}

void ParticleSystem::UpdateVertexBuffer(ID3D11DeviceContext* context)
{
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource = {};
		context->Map(m_vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		memcpy(mappedResource.pData, &m_vertices[0], m_vertices.size());
		context->Unmap(m_vertexBuffer.Get(), 0);
	}
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource = {};
		context->Map(m_indexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		memcpy(mappedResource.pData, &m_indices[0], m_indices.size());
		context->Unmap(m_indexBuffer.Get(), 0);
	}
}

Particle::Particle(ParticleType type, const Vec3D& accel, const Vec3D& initVel, const Vec3D& initPos, const float lifespan):
	m_accel{accel},
	m_initVel{initVel},
	m_initPos{initPos},
	m_pos{},
	m_lifespan{lifespan},
	m_type{type}
{
}

void Particle::CreateQuad(int width, int height, const Vec3D& up, const Vec3D& right)
{
	Vec3D R = right;
	MathVec3DNormalize(&R);
	Vec3D U = up;
	MathVec3DNormalize(&U);
	const Vec3D P = m_pos;

	Vec3D X = (width / 2.0f) * R;
	Vec3D Y = (width / 2.0f) * U;
	const Vec3D Q1 = P + X + Y;
	const Vec3D Q4 = P - X + Y; // q4
	const Vec3D Q3 = P - X - Y; // q3
	const Vec3D Q2 = P + X - Y; // q2

	m_vertices[0].Position = Q1;
	m_vertices[1].Position = Q2;
	m_vertices[2].Position = Q3;
	m_vertices[3].Position = Q4;
	m_vertices[0].Lifespan = m_lifespan;
	m_vertices[1].Lifespan = m_lifespan;
	m_vertices[2].Lifespan = m_lifespan;
	m_vertices[3].Lifespan = m_lifespan;

	m_indices[0] = 0;
	m_indices[1] = 1;
	m_indices[2] = 2;
	m_indices[3] = 2;
	m_indices[4] = 3;
	m_indices[5] = 0;
}
