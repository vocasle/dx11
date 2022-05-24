#include <d3d11.h>
#include <wrl/client.h>

#include <vector>
#include <string>

#include "Math.h"
#include "Camera.h"

class ParticleSystem
{
private:
	struct Vertex
	{
		Vec3D Position;
		float Lifespan;
	};

	class Emitter;
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
		Vertex m_vertices[4];
		uint32_t m_indices[6];

		friend class Emitter;
	};

	class Emitter
	{
	public:
		void Tick(const float deltaTime);
		Emitter(int particlesPerTick, const Vec3D& pos, const Vec3D& initVelocity, const Vec3D& accel);
		void SetParticles(std::vector<Particle>* particles);
		void SetCamera(Camera* camera);
		
	private:
		Particle EmitParticle(int seed);

		float m_lifespan;
		int m_particlesPerTick;
		Vec3D m_pos;
		Vec3D m_initVelocity;
		Vec3D m_acceleration;

		std::vector<Particle>* m_particles;
		const Camera* m_camera;
	};

public:
	ParticleSystem(const std::string& name);
	~ParticleSystem();

	void Init(ID3D11Device* device, ID3D11DeviceContext* context, const std::string& texFilePath);
	void Reset();
	void Destroy();
	void Tick(const float deltaTime);

	void SetBlendState(ID3D11BlendState* blendState);
	void SetDepthStencilState(ID3D11DepthStencilState* depthStencilState);
	void SetOrigin(const Vec3D& origin);
	void SetCamera(const Camera& camera);

	ID3D11BlendState* GetBlendState() const { return m_blendState.Get(); }
	ID3D11DepthStencilState* GetDepthStencilState() const { return m_depthStencilState.Get(); }
	ID3D11Buffer* GetVertexBuffer() const { return m_vertexBuffer.Get(); }
	ID3D11Buffer* GetIndexBuffer() const { return m_indexBuffer.Get(); }
	size_t GetNumIndices() const { return m_particles.size() * 6; }
	size_t GetStrideSize() const { return sizeof(Particle); }
	std::string GetName() const { return m_name; }

private:
	void CreateTexture(ID3D11Device* device, ID3D11DeviceContext* context, const std::string& filepath);
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
	std::vector<Emitter> m_emitters;
	std::vector<Vertex> m_vertices;
	std::vector<uint32_t> m_indices;

	Vec3D m_origin;
	const Camera* m_camera;

	static const int MAX_LIFESPAN = 5;
	static const int MAX_PARTICLES = 10;
};
