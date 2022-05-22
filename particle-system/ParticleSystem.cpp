#include "ParticleSystem.h"
#include "Image.h"

ParticleSystem::ParticleSystem()
{
}

ParticleSystem::~ParticleSystem()
{
}

void ParticleSystem::Init(ID3D11Device* device, const std::string& texFilePath)
{
	
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
