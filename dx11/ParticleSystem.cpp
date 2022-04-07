#include "ParticleSystem.h"
#include "Utils.h"

ParticleSystem::ParticleSystem():
	m_vb{},
	m_particles{},
	m_ps{},
	m_gs{},
	m_vs{}
{
}

ParticleSystem::~ParticleSystem()
{
}

void ParticleSystem::Init(ID3D11Device* device)
{
	m_ps = Shader{ "ParticlePS.cso", device, ShaderType::Pixel };
	m_gs = Shader{ "ParticleGS.cso", device, ShaderType::Geometry };
	m_vs = Shader{ "ParticleVS.cso", device, ShaderType::Vertex };

	{
		D3D11_BUFFER_DESC desc = {};
		desc.ByteWidth = sizeof(Particle) * m_particles.size();
	}
}

void ParticleSystem::InitParticles()
{
}
