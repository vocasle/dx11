#include "ParticleSystem.h"
#include "Image.h"

using namespace Microsoft::WRL;

ParticleSystem::ParticleSystem(const std::string& name): m_name{name}
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
	CreateVertexBuffer(device);
	CreateIndexBuffer(device);
	CreateSamplerState(device);
}

void ParticleSystem::Reset()
{
}

void ParticleSystem::Destroy()
{
}

void ParticleSystem::Tick(const float deltaTime)
{
	for (Particle& p : m_particles)
	{
		p.Tick(deltaTime);
	}
}

void ParticleSystem::SetBlendState(ID3D11BlendState* blendState)
{
	m_blendState = blendState;
}

void ParticleSystem::SetDepthStencilState(ID3D11DepthStencilState* depthStencilState)
{
	m_depthStencilState = depthStencilState;
}

void ParticleSystem::SetOrigin(const Vec3D& origin)
{
	m_origin = origin;
}

ParticleSystem::Particle::Particle(const Vec3D& acceleration, 
	const Vec3D& initVelocity, 
	const Vec3D& initPos, 
	const float lifespan):
	m_acceleration{acceleration},
	m_initVelocity{initVelocity},
	m_initPos{initPos},
	m_pos{},
	m_lifespan{lifespan}
{
}

void ParticleSystem::Particle::Tick(const float deltaTime)
{
	// p(t) = 1/2 t*t * a + t * v0 + p0
	// a - acceleration
	// v0 - init velocity
	// p0 - init position
	// t - total time in seconds
	m_lifespan += deltaTime;
	m_pos = (0.5f * m_lifespan * m_lifespan) * m_acceleration + m_lifespan * m_initVelocity + m_initPos;
}

bool ParticleSystem::Particle::IsAlive() const
{
	return m_lifespan <= static_cast<float>(MAX_LIFESPAN);
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
	UtilsCreateVertexBuffer(device, &m_vertices[0], m_vertices.size(), sizeof(Vertex), m_vertexBuffer.ReleaseAndGetAddressOf());
}

void ParticleSystem::CreateIndexBuffer(ID3D11Device* device)
{
	UtilsCreateIndexBuffer(device, &m_indices[0], m_indices.size(), m_indexBuffer.ReleaseAndGetAddressOf());
}

void ParticleSystem::CreateSamplerState(ID3D11Device* device)
{
	CD3D11_SAMPLER_DESC desc = CD3D11_SAMPLER_DESC{ CD3D11_DEFAULT{} };
	HR(device->CreateSamplerState(&desc, m_sampler.ReleaseAndGetAddressOf()))
}
