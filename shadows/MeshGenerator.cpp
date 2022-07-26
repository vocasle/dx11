#include "MeshGenerator.h"
#include "Actor.h"

#include <cassert>

HRESULT
ComputeTangentFrame(const std::vector<uint32_t> &indices,
		    std::vector<Vertex> &vertices)
{
	static constexpr float EPSILON = 0.0001f;
	static const Vec4D s_flips = { 1.f, -1.f, -1.f, 1.f };
	assert(indices.size() % 3 == 0);

	std::vector<Vec3D> tangent1(vertices.size());
	std::vector<Vec3D> tangent2(vertices.size());

	for (size_t face = 0; face < indices.size() / 3; ++face) {
		uint32_t i0 = indices[face * 3];
		uint32_t i1 = indices[face * 3 + 1];
		uint32_t i2 = indices[face * 3 + 2];

		if (i0 == uint32_t(-1) || i1 == uint32_t(-1) ||
		    i2 == uint32_t(-1))
			continue;

		if (i0 >= vertices.size() || i1 >= vertices.size() ||
		    i2 >= vertices.size())
			return E_UNEXPECTED;

		const Vec2D t0 = vertices[i0].TexCoords;
		const Vec2D t1 = vertices[i1].TexCoords;
		const Vec2D t2 = vertices[i2].TexCoords;
		Vec2D d1;
		Vec2D d2;
		MathVec2DSubtraction(&t1, &t0, &d1);
		MathVec2DSubtraction(&t2, &t0, &d2);

		Vec4D s = { d1.X, d2.X, d1.Y, d2.Y };

		Vec4D tmp = s;

		float d = tmp.X * tmp.W - tmp.Z * tmp.Y;
		d = (fabsf(d) <= EPSILON) ? 1.f : (1.f / d);
		MathVec4DModulateByScalar(&s, d, &s);
		MathVec4DModulateByVec4D(&s, &s_flips, &s);

		Mat4X4 m0;
		const Vec4D zero = { 0.0f, 0.0f, 0.0f, 0.0f };
		m0.V[0] = MathVec4DVectorPermute(s, zero, 3, 2, 6, 7);
		m0.V[1] = MathVec4DVectorPermute(s, zero, 1, 0, 4, 5);
		m0.V[2] = m0.V[3] = zero;

		const Vec3D p0 = vertices[i0].Position;
		const Vec3D p1 = vertices[i1].Position;
		Vec3D p2 = vertices[i2].Position;

		Mat4X4 m1;
		m1.V[0] = Vec4D(MathVec3DSubtraction(&p1, &p0), 0.0f);
		m1.V[1] = Vec4D(MathVec3DSubtraction(&p2, &p0), 0.0f);
		m1.V[2] = m1.V[3] = zero;

		const Mat4X4 uv = MathMat4X4MultMat4X4ByMat4X4(&m0, &m1);
		Vec4D sum =
			MathVec4DAddition(Vec4D(tangent1[i0], 1.0f), uv.V[0]);
		tangent1[i0] = { sum.X, sum.Y, sum.Z };
		sum = MathVec4DAddition(Vec4D(tangent1[i1], 1.0f), uv.V[0]);
		tangent1[i1] = { sum.X, sum.Y, sum.Z };
		sum = MathVec4DAddition(Vec4D(tangent1[i2], 1.0f), uv.V[0]);
		tangent1[i2] = { sum.X, sum.Y, sum.Z };

		sum = MathVec4DAddition(Vec4D(tangent2[i0], 1.0f), uv.V[1]);
		tangent2[i0] = { sum.X, sum.Y, sum.Z };
		sum = MathVec4DAddition(Vec4D(tangent2[i1], 1.0f), uv.V[1]);
		tangent2[i1] = { sum.X, sum.Y, sum.Z };
		sum = MathVec4DAddition(Vec4D(tangent2[i2], 1.0f), uv.V[1]);
		tangent2[i2] = { sum.X, sum.Y, sum.Z };
	}

	for (size_t j = 0; j < vertices.size(); ++j) {
		// Gram-Schmidt orthonormalization
		Vec3D b0 = vertices[j].Normal;
		MathVec3DNormalize(&b0);

		const Vec3D tan1 = tangent1[j];

		Vec3D b1 = MathVec3DSubtraction(
			tan1, MathVec3DModulateByScalar(
				      &b0, MathVec3DDot(&b0, &tan1)));
		MathVec3DNormalize(&b1);

		const Vec3D tan2 = tangent2[j];
		Vec3D b2 = MathVec3DSubtraction(
			MathVec3DSubtraction(
				tan2, MathVec3DModulateByScalar(
					      &b0, MathVec3DDot(&b0, &tan2))),
			MathVec3DModulateByScalar(&b1,
						  MathVec3DDot(&b1, &tan2)));
		MathVec3DNormalize(&b2);

		// handle degenerate vectors

		const float len1 = MathVec3DLength(b1);
		const float len2 = MathVec3DLength(b2);

		if ((len1 <= EPSILON) || (len2 <= EPSILON)) {
			if (len1 > 0.5f) {
				// Reset bi-tangent from tangent and normal
				b2 = MathVec3DCross(&b0, &b1);
			} else if (len2 > 0.5f) {
				// Reset tangent from bi-tangent and normal
				b1 = MathVec3DCross(&b2, &b0);
			} else {
				// Reset both tangent and bi-tangent from normal
				Vec3D axis;
				const Vec3D identityR0 = { 1.0f, 0.0f, 0.0f };
				const Vec3D identityR1 = { 0.0f, 1.0f, 0.0f };
				const Vec3D identityR2 = { 0.0f, 0.0f, 1.0f };
				const float d0 =
					fabsf(MathVec3DDot(&identityR0, &b0));
				const float d1 =
					fabsf(MathVec3DDot(&identityR1, &b0));
				const float d2 =
					fabsf(MathVec3DDot(&identityR2, &b0));
				if (d0 < d1) {
					axis = (d0 < d2) ? identityR0 :
							   identityR1;
				} else if (d1 < d2) {
					axis = identityR1;
				} else {
					axis = identityR2;
				}

				b1 = MathVec3DCross(&b0, &axis);
				b2 = MathVec3DCross(&b0, &b1);
			}
		}
		vertices[j].Tangent = b1;
		vertices[j].Bitangent = b2;
	}

	return S_OK;
}