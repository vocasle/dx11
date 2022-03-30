#pragma once

#include "Math.h"
#include "Camera.h"

#include <d3d11.h>
#include <stdint.h>

class ParticleSystem
{
public:
	ParticleSystem();
	~ParticleSystem();

	float GetAge() const;

	void SetCameraPos(const Vec3D& camPosW);
	void SetEmitPos(const Vec3D& emitPosW);
	void SetEmitDir(const Vec3D& emitDirW);

	void Init(ID3D11Device* device,
		ID3D11PixelShader* ps,
		ID3D11ShaderResourceView* texArraySRV,
		ID3D11ShaderResourceView* randomTexSRV,
		uint32_t maxParcticles);

	void Reset();
	void Update(float dt, float gameTime);
	void Draw(ID3D11DeviceContext* ctx, const Camera& cam);
private:

	void BuildVB(ID3D11Device* device);

	ParticleSystem(const ParticleSystem& rhs);
	ParticleSystem& operator=(const ParticleSystem& rhs);

private:

	uint32_t mMaxParticles;
	bool mIsFirstRun;

	float mGameTime;
	float mTimeStep;
	float mAge;

	Vec3D mCamPosW;
	Vec3D mEmitPosW;
	Vec3D mEmitDirW;

	ID3D11PixelShader* mPS;

	ID3D11Buffer* mInitVB;
	ID3D11Buffer* mDrawVB;
	ID3D11Buffer* mStreamOutVB;

	ID3D11ShaderResourceView* mTexArraySRV;
	ID3D11ShaderResourceView* mRandomTexSRV;

};
