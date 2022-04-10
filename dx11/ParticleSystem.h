#pragma once

#include "Math.h"
#include "Camera.h"
#include "Shader.h"
#include "D3DCommonTypes.h"

#include <d3d11.h>
#include <stdint.h>
#include <vector>

class ParticleSystem
{
private:
	struct Particle
	{
		Particle() : InitPosW{}, InitVelW{}, SizeW{}, Age{ 0.0f } {}
		Vec3D InitPosW;
		Vec3D InitVelW;
		Vec2D SizeW;
		float Age;
	};

	struct PerFrameConstants
	{
		Mat4X4 Proj;
		Mat4X4 WorldInvTranspose;
		Mat4X4 World;
		Mat4X4 View;
		Vec3D CamPosW;
		float _Pad;
	};

public:
	ParticleSystem();
	~ParticleSystem();

	void Init(ID3D11Device* device, ID3D11DeviceContext* context);
	void Draw(ID3D11DeviceContext* context);
	void Update(const Mat4X4& inView,
		const Mat4X4& inProj,
		const Mat4X4& inWorld,
		const Vec3D& inCamPosW,
		double inDelta,
		double inGameTime);
	void SetSamplerState(ID3D11SamplerState* inSampler);

private:
	void InitParticles();
	void CreateInputLayout(ID3D11Device* device);
	void UpdateParticles(double inDelta);
	void ResetParticle(Particle& p);

private:

	ID3D11Buffer* m_vb;
	std::vector<Particle> m_particles;
	Shader m_ps;
	Shader m_gs;
	Shader m_vs;
	ID3D11InputLayout* m_inputLayout;
	ID3D11Buffer* m_cb;
	PerFrameConstants m_perFrameConstants;
	Texture m_texture;
	Texture m_velocityTexture;
	ID3D11BlendState* m_blendState;
	ID3D11SamplerState* m_sampler;
	ID3D11DepthStencilState* m_depthStencilState;

	static constexpr float MAX_AGE = 0.5f;
	static constexpr uint32_t MAX_PARTICLES = 200;

};
