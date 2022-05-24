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
	CreateEmitter();
}

void ParticleSystem::Reset()
{
}

void ParticleSystem::Destroy()
{
}

void ParticleSystem::Tick(const float deltaTime)
{
	for (Emitter& e : m_emitters)
	{
		e.Tick(deltaTime);
	}

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

void ParticleSystem::SetCamera(const Camera& camera)
{
	m_camera = &camera;
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

void ParticleSystem::CreateEmitter()
{
	Emitter emitter(5, m_origin, { 0.0f, 0.0f, 0.0f }, { 0.0f, -9.8f, 0.0f });
	m_emitters.push_back(emitter);
}

void ParticleSystem::Emitter::Tick(const float deltaTime)
{
	m_lifespan += deltaTime;

	for (int i = 0; i < m_particlesPerTick && m_particlesPerTick + m_particles->size() < MAX_PARTICLES; ++i)
	{
		m_particles->push_back(EmitParticle(i));
	}

	for (int i = 0; i < m_particles->size(); ++i)
	{
		Particle& p = m_particles->at(i);
		if (!p.IsAlive())
		{
			p = EmitParticle(i);
		}
	}
}

ParticleSystem::Emitter::Emitter(int particlesPerTick, const Vec3D& pos, const Vec3D& initVelocity, const Vec3D& accel)
	: m_lifespan{0.0f},
	m_particlesPerTick(particlesPerTick),
	m_pos(pos),
	m_initVelocity(initVelocity),
	m_acceleration(accel),
	m_particles{nullptr}
{
}

void ParticleSystem::Emitter::SetParticles(std::vector<Particle>* particles)
{
	m_particles = particles;
}

void ParticleSystem::Emitter::SetCamera(Camera* camera)
{
	m_camera = camera;
}

ParticleSystem::Particle ParticleSystem::Emitter::EmitParticle(int seed)
{
	Particle particle(m_acceleration, m_initVelocity, m_pos, 5.0f);
	int width = 5;
	int height = 5;

	Vec3D R = m_camera->GetRight();
	MathVec3DNormalize(&R);
	Vec3D U = m_camera->GetUp();
	MathVec3DNormalize(&U);
	const Vec3D P = m_pos;

	Vec3D X = (width / 2.0f) * R;
	Vec3D Y = (height / 2.0f) * U;
	const Vec3D Q1 = P + X + Y;
	const Vec3D Q4 = P - X + Y; // q4
	const Vec3D Q3 = P - X - Y; // q3
	const Vec3D Q2 = P + X - Y; // q2


	return particle;
}
