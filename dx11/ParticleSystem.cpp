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
	m_cb{nullptr},
	m_texture{},
	m_velocityTexture{},
	m_blendState{nullptr},
	m_sampler{nullptr},
	m_depthStencilState{nullptr}
{
}

ParticleSystem::~ParticleSystem()
{
	COM_FREE(m_inputLayout);
	COM_FREE(m_cb);
	COM_FREE(m_blendState);
}

void ParticleSystem::Init(ID3D11Device* device, ID3D11DeviceContext* context)
{
	m_ps = Shader{ "ParticlePS.cso", device, ShaderType::Pixel };
	m_gs = Shader{ "ParticleGS.cso", device, ShaderType::Geometry };
	m_vs = Shader{ "ParticleVS.cso", device, ShaderType::Vertex };

	InitParticles();

	{
		D3D11_BUFFER_DESC desc = {};
		desc.ByteWidth = sizeof(Particle) * MAX_PARTICLES;
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

	D3DHelper::LoadTextureFromFile(device, context, "assets/textures/flare0.dds", &m_texture);

	m_velocityTexture.SRV = D3DHelper::CreateRandomTexture1DSRV(device);

	{
		D3D11_BLEND_DESC transparentDesc = {};
		transparentDesc.AlphaToCoverageEnable = false;
		transparentDesc.IndependentBlendEnable = false;
		transparentDesc.RenderTarget[0].BlendEnable = true;
		transparentDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		transparentDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		transparentDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		transparentDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		transparentDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
		transparentDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		transparentDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		if (FAILED(device->CreateBlendState(&transparentDesc, &m_blendState)))
		{
			UTILS_FATAL_ERROR("Failed to create blend state");
		}
	}

	{
		D3D11_DEPTH_STENCIL_DESC desc = {};
		desc.DepthEnable = false;

		HR(device->CreateDepthStencilState(&desc, &m_depthStencilState));
	}
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
	context->PSSetSamplers(0, 1, &m_sampler);
	context->PSSetShaderResources(0, 1, &m_texture.SRV);
	constexpr float blendFactors[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	context->OMSetBlendState(m_blendState, blendFactors, 0xffffffff);
	context->OMSetDepthStencilState(m_depthStencilState, 0);

	context->Draw(m_particles.size(), 0);
}

void ParticleSystem::Update(const Mat4X4& inView,
	const Mat4X4& inProj,
	const Mat4X4& inWorld,
	const Vec3D& inCamPosW, 
	double inDelta,
	double inGameTime)
{
	//UtilsDebugPrint("delta: %f, game: %f\n", inDelta, inGameTime);
	m_perFrameConstants.CamPosW = inCamPosW;
	m_perFrameConstants.Proj = inProj;
	m_perFrameConstants.WorldInvTranspose = MathMat4X4Inverse(&inWorld);
	MathMat4X4Transpose(&m_perFrameConstants.World);
	m_perFrameConstants.World = inWorld;
	m_perFrameConstants.View = inView;
	UpdateParticles(inDelta);
}

void ParticleSystem::SetSamplerState(ID3D11SamplerState* inSampler)
{
	m_sampler = inSampler;
}

void ParticleSystem::InitParticles()
{
	m_particles.reserve(MAX_PARTICLES);
	Particle p = {};
	
	//for (uint32_t i = 0; i < MAX_PARTICLES; ++i)
	{
		ResetParticle(p);
		m_particles.push_back(p);
	}
}

void ParticleSystem::CreateInputLayout(ID3D11Device* device)
{
	D3D11_INPUT_ELEMENT_DESC desc[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"VELOCITY", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"AGE", 0, DXGI_FORMAT_R32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0},
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

		if (p.Age > MAX_AGE)
		{
			ResetParticle(p);
		}
	}

	if (m_particles.size() /*+ 5*/ < MAX_PARTICLES)
	{
		//for (uint32_t i = 0; i < 5; ++i)
		{
			m_particles.emplace_back();
		}
	}
}

void ParticleSystem::ResetParticle(Particle& p)
{
	p.InitPosW = { 0.0f, 0.0f , 0.0f };
	p.SizeW = { 5.0f, 5.0f };
	p.Age = 0.0f;
	p.InitVelW = { MathRandom(-1.0f, 1.0f), MathRandom(0.0f, 1.0f), MathRandom(-1.0f, 1.0f) };
	MathVec3DNormalize(&p.InitVelW);
	p.InitVelW = MathVec3DModulateByScalar(&p.InitVelW, 1.0f);
}