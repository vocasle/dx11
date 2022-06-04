struct VSIn
{
	float3 Position : POSITION;
};

struct PSIn
{
	float4 PositionH : SV_POSITION;
};

PSIn main(VSIn vin)
{
	PSIn pin;
	float4 i = float4(1.0f, 0.0f, 0.0f, 0.0f);
	float4 j = float4(0.0f, 1.0f, 0.0f, 0.0f);
	float4 k = float4(0.0f, 0.0f, 1.0f, 0.0f);
	float4 u = float4(2.0f, 2.0f, 10.0f, 1.0f);
	float4x4 world = {i,j,k,u};

	static const float PI = 3.14159265f;
	float fov_hor = PI / 4.0f;
	float fov_ver = PI / 4.0f;
	float w = 1.0f / tan(fov_hor * 0.5f);
	float h = 1.0f / tan(fov_ver * 0.5f);
	float far_plane = 100.0f;
	float near_plane = 0.1f;
	float Q = far_plane / (far_plane - near_plane);
	float4x4 proj = {
		w, 0.0f, 0.0f, 0.0f,
		0.0f, h, 0.0f, 0.0f,
		0.0f, 0.0f, Q, 1.0f,
		0.0f, 0.0f, -Q * near_plane, 0.0f
	};
	float4x4 M = mul(world, proj);
	pin.PositionH = mul(float4(vin.Position, 1.0f), M);
	return pin;
}
