#include <d3d11.h>
#include <wrl/client.h>

#include <vector>
#include <string>

#include "Math.h"

class ParticleSystem
{
private:
	struct Vertex
	{
		Vec3D Position;
		float Lifespan;
	};

	class Particle
	{
	public:
		Particle(const Vec3D& acceleration, const Vec3D& initVelocity, const Vec3D& initPos, const float lifespan);
		void Tick(const float deltaTime);
		bool IsAlive() const;

	private:
		Vec3D m_acceleration;
		Vec3D m_initVelocity;
		Vec3D m_initPos;
		Vec3D m_pos;
		float m_lifespan;
	};

public:
	ParticleSystem();
	~ParticleSystem();

	void Init(ID3D11Device* device, const std::string& texFilePath);
	void Reset();
	void Destroy();
	void Tick(const float deltaTime);

	void SetBlendState(ID3D11BlendState* blendState);
	void SetDepthStencilState(ID3D11DepthStencilState* depthStencilState);
	void SetOrigin(const Vec3D& origin);

	ID3D11BlendState* GetBlendState() const { return m_blendState.Get(); }
	ID3D11DepthStencilState* GetDepthStencilState() const { return m_depthStencilState.Get(); }
	ID3D11Buffer* GetVertexBuffer() const { return m_vertexBuffer.Get(); }
	ID3D11Buffer* GetIndexBuffer() const { return m_indexBuffer.Get(); }
	size_t GetNumIndices() const { return m_particles.size() * 6; }
	size_t GetStrideSize() const { return sizeof(Particle); }

private:
	void CreateTexture(ID3D11Device* device, const std::string& filepath);
	void CreateBlendState(ID3D11Device* device);
	void CreateDepthStencilState(ID3D11Device* device);
	void CreateVertexBuffer(ID3D11Device* device);
	void CreateIndexBuffer(ID3D11Device* device);
	void CreateSamplerState(ID3D11Device* device);
	void CreateEmitter();

private:
	Microsoft::WRL::ComPtr<ID3D11BlendState> m_blendState;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStencilState;

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_diffuseTexture;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_sampler;

	std::string m_name;
	std::vector<Particle> m_particles;
	std::vector<Vertex> m_vertices;

	Vec3D m_origin;

	static const int MAX_LIFESPAN = 5;
	static const int MAX_PARTICLES = 10;
};
