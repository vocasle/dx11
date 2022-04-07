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
		float Size;
	};

	struct PerFrameConstants
	{
		Mat4X4 View;
		Mat4X4 Proj;
		Mat4X4 World;
		Vec3D CamPosW;
	};

public:
	ParticleSystem();
	~ParticleSystem();

	void Init(ID3D11Device* device);
	void Draw(ID3D11DeviceContext* context);

private:
	void InitParticles();

private:

	ID3D11Buffer* m_vb;
	std::vector<Particle> m_particles;
	Shader m_ps;
	Shader m_gs;
	Shader m_vs;
	//static constexpr


};
