#pragma once

#include "Math.h"
#include "Camera.h"
#include "Shader.h"

#include <d3d11.h>
#include <stdint.h>
#include <vector>

class ParticleSystem
{
private:
	struct Particle
	{
		Vec3D Position;
		Vec3D Velocity;
		float Age;
		Vec2D Size;
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

	void Init(ID3D11Device* device);
	void Draw(ID3D11DeviceContext* context);
	void Update(const Mat4X4& inView,
		const Mat4X4& inProj,
		const Mat4X4& inWorld,
		const Vec3D& inCamPosW,
		double inDelta,
		double inGameTime);

private:
	void InitParticles();
	void CreateInputLayout(ID3D11Device* device);
	void UpdateParticles(double inDelta);
	void ResetParticle(Particle& p);
	void UpdateParticle(Particle& p, float t);

private:

	ID3D11Buffer* m_vb;
	std::vector<Particle> m_particles;
	Shader m_ps;
	Shader m_gs;
	Shader m_vs;
	ID3D11InputLayout* m_inputLayout;
	ID3D11Buffer* m_cb;
	PerFrameConstants m_perFrameConstants;

	static constexpr float MAX_AGE = 2.0f;
	static constexpr uint32_t MAX_PARTICLES = 1000;

};
