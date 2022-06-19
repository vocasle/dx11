#include <d3d11.h>
#include <wrl/client.h>

#include <vector>
#include <string>
#include <array>

#include "NE_Math.h"
#include "Camera.h"

enum class ParticleType
{
	Particle,
	Emitter
};

class Particle
{
public:
	struct Vertex
	{
		Vec3D Position;
		Vec2D TexCoords;
		float Lifespan;
	};

public:
	Particle(ParticleType type, const Vec3D& accel, const Vec3D& initVel, const Vec3D& initPos, const float lifespan);
	void CreateQuad(int width, int height, const Vec3D& up, const Vec3D& right);
	const Vec3D& GetAccel() const { return m_accel; }
	const Vec3D& GetInitVel() const { return m_initVel; }
	const Vec3D& GetInitPos() const { return m_initPos; }
	const Vec3D& GetPos() const { return m_pos; }
	float GetLifespan() const { return m_lifespan; }
	ParticleType GetParticleType() const { return m_type; }
	const std::array<Vertex, 4>& GetVertices() const { return m_vertices; }
	const std::array<uint32_t, 6>& GetIndices() const { return m_indices; }
	void Tick(const float deltaTime);
	bool IsAlive() const;
	void Reset();

private:
	Vec3D m_accel;
	Vec3D m_initVel;
	Vec3D m_initPos;
	Vec3D m_pos;
	float m_lifespan;
	ParticleType m_type;
	std::array<Vertex, 4> m_vertices;
	std::array<uint32_t, 6> m_indices;
};


class ParticleSystem
{
public:
	static const int MAX_LIFESPAN = 1;
	static const int MAX_PARTICLES = 70;
	static const int PARTICLE_SIZE = 1;

public:
	ParticleSystem(const std::string& name, const Vec3D& origin, const Camera& camera);
	~ParticleSystem();

	void Init(ID3D11Device* device, ID3D11DeviceContext* context, const std::string& texFilePath);
	void Tick(const float deltaTime);
	void UpdateVertexBuffer(ID3D11DeviceContext* context);

	ID3D11BlendState* GetBlendState() const { return m_blendState.Get(); }
	ID3D11DepthStencilState* GetDepthStencilState() const { return m_depthStencilState.Get(); }
	ID3D11Buffer* GetVertexBuffer() const { return m_vertexBuffer.Get(); }
	ID3D11Buffer* GetIndexBuffer() const { return m_indexBuffer.Get(); }
	size_t GetNumIndices() const { return m_indices.size(); }
	std::string GetName() const { return m_name; }
	ID3D11ShaderResourceView* GetSRV() const { return m_diffuseTexture.Get(); }
	ID3D11SamplerState* GetSamplerState() const { return m_sampler.Get(); }

	size_t GetNumAliveParticles() const;

private:
	void CreateTexture(ID3D11Device* device, ID3D11DeviceContext* context, const std::string& filepath);
	void CreateBlendState(ID3D11Device* device);
	void CreateDepthStencilState(ID3D11Device* device);
	void CreateVertexBuffer(ID3D11Device* device);
	void CreateIndexBuffer(ID3D11Device* device);
	void CreateSamplerState(ID3D11Device* device);
	void UpdateVertices();
	void CreateEmitter();
	void EmitParticle();

private:
	Microsoft::WRL::ComPtr<ID3D11BlendState> m_blendState;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStencilState;

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_diffuseTexture;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_sampler;

	std::string m_name;
	std::vector<Particle> m_particles;
	std::vector<Particle::Vertex> m_vertices;
	std::vector<uint32_t> m_indices;
	Particle m_emitter;

	Vec3D m_origin;
	const Camera* m_camera;
};
