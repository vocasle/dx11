#include "ParticleSystem.h"
#include "Utils.h"
#include "D3DHelper.h"

ParticleSystem::ParticleSystem():
	m_vb{},
	m_particles{},
	m_ps{},
	m_gs{},
	m_vs{},
	m_inputLayout{nullptr},
	m_cb{nullptr}
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

	InitParticles();

	{
		D3D11_BUFFER_DESC desc = {};
		desc.ByteWidth = sizeof(Particle) * m_particles.size();
		desc.StructureByteStride = sizeof(Particle);
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = &m_particles[0];
		
		if (FAILED(device->CreateBuffer(&desc, &initData, &m_vb)))
		{
			UTILS_FATAL_ERROR("Failed to create particle system vertex buffer");
		}
	}

	CreateInputLayout(device);

	D3DHelper::CreateConstantBuffer(device, sizeof(PerFrameConstants), &m_cb);
}

void ParticleSystem::Draw(ID3D11DeviceContext* context)
{
	D3DHelper::UpdateConstantBuffer(context, sizeof(PerFrameConstants), &m_perFrameConstants, m_cb);

	{
		D3D11_MAPPED_SUBRESOURCE resource = {};
		context->Map(m_vb, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
		memcpy(resource.pData, &m_particles[0], sizeof Particle * m_particles.size());
		context->Unmap(m_vb, 0);
	}

	context->VSSetShader(m_vs.GetAs<ID3D11VertexShader*>(), nullptr, 0);
	context->PSSetShader(m_ps.GetAs<ID3D11PixelShader*>(), nullptr, 0);
	context->GSSetShader(m_gs.GetAs<ID3D11GeometryShader*>(), nullptr, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	context->IASetInputLayout(m_inputLayout);
	uint32_t strides = sizeof(Particle);
	uint32_t offsets = 0;
	context->IASetVertexBuffers(0, 1, &m_vb, &strides, &offsets);
	context->GSSetConstantBuffers(0, 1, &m_cb);

	context->Draw(m_particles.size(), 0);
}

void ParticleSystem::Update(const Mat4X4& inView,
	const Mat4X4& inProj,
	const Mat4X4& inWorld,
	const Vec3D& inCamPosW, 
	double inDelta)
{
	m_perFrameConstants.CamPosW = inCamPosW;
	m_perFrameConstants.Proj = inProj;
	m_perFrameConstants.WorldInvTranspose = MathMat4X4Inverse(&inWorld);
	MathMat4X4Transpose(&m_perFrameConstants.World);
	m_perFrameConstants.World = inWorld;
	m_perFrameConstants.View = inView;
	UpdateParticles(inDelta);
}

void ParticleSystem::InitParticles()
{
	for (uint32_t i = 0; i < 10; ++i)
	{
		Particle p = {};
		const float size = MathRandom(0.1f, 1.0f);
		p.Position.X = MathRandom(-5.0f, 5.0f);
		p.Position.Y = MathRandom(-5.0f, 5.0f);
		p.Position.Z = MathRandom(-5.0f, 5.0f);
		p.Size.X = size;
		p.Size.Y = size;
		m_particles.push_back(p);
	}
}

void ParticleSystem::CreateInputLayout(ID3D11Device* device)
{
	D3D11_INPUT_ELEMENT_DESC desc[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"VELOCITY", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"AGE", 0, DXGI_FORMAT_R32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	if (FAILED(device->CreateInputLayout(desc, 4, m_vs.GetByteCode(), m_vs.GetByteCodeLen(), &m_inputLayout)))
	{
		UTILS_FATAL_ERROR("Failed to create particle system input layout");
	}
}

void ParticleSystem::UpdateParticles(double inDelta)
{
	const float delta = static_cast<float>(inDelta / 1000.0);
	for (Particle& p : m_particles)
	{
		p.Age += delta;
		p.Position.Y += delta;

		if (p.Age > 1.0f)
		{
			ResetParticle(p);
		}
	}
}

void ParticleSystem::ResetParticle(Particle& p)
{
	p.Age = 0.0f;
	p.Position = { MathRandom(-5.0f, 5.0f), MathRandom(-5.0f, 5.0f), MathRandom(-5.0f, 5.0f) };
	const float size = MathRandom(0.1f, 1.0f);
	p.Size = { size, size };
	p.Velocity = { 0.0f, 0.0f, 0.0f };
}
