#include "MeshGenerator.h"

#include <DirectXMath.h>
#include <memory>
#include <algorithm>
#include <iterator>

#define HRESULT_E_ARITHMETIC_OVERFLOW static_cast<HRESULT>(0x80070216L)


using namespace DirectX;

HRESULT ComputeTangentFrameImpl(
    const std::vector<uint32_t>& indices,
    size_t nFaces,
    const std::vector<XMFLOAT3>& positions,
    const std::vector<XMFLOAT3>& normals,
    const std::vector<XMFLOAT2>& texcoords,
    size_t nVerts,
    std::vector<XMFLOAT3>& tangents3,
    std::vector<XMFLOAT3>& bitangents);

HRESULT ComputeTangentFrame(
    const std::vector<uint32_t>& indices,
    size_t nFaces,
    const std::vector<Vec3D>& positions,
    const std::vector<Vec3D>& normals,
    const std::vector<Vec2D>& texcoords,
    size_t nVerts,
    std::vector<Vec3D>& tangents,
    std::vector<Vec3D>& bitangents)
{
    std::vector<XMFLOAT3> xmPositions;
    std::transform(std::begin(positions),
        std::end(positions),
        std::back_inserter(xmPositions),
        [](const Vec3D& v) { return XMFLOAT3{ v.X, v.Y, v.Z }; });

    std::vector<XMFLOAT3> xmNormals(nVerts);
    std::transform(std::begin(normals),
        std::end(normals),
        std::back_inserter(xmNormals),
        [](const Vec3D& v) { return XMFLOAT3{ v.X, v.Y, v.Z }; });

    std::vector<XMFLOAT2> xmTexCoords(nVerts);
    std::transform(std::begin(texcoords),
        std::end(texcoords),
        std::back_inserter(xmTexCoords),
        [](const Vec2D& v) { return XMFLOAT2{ v.X, v.Y }; });

    std::vector<XMFLOAT3> xmTangents;
    std::vector<XMFLOAT3> xmBitangents;

    HRESULT hr = ComputeTangentFrameImpl(indices, nFaces, xmPositions, xmNormals, xmTexCoords, nVerts, xmTangents, xmBitangents);

    std::transform(std::begin(xmTangents),
        std::end(xmTangents),
        std::back_inserter(tangents),
        [](const XMFLOAT3& v) { return Vec3D{ v.x, v.y, v.z }; });

    std::transform(std::begin(xmBitangents),
        std::end(xmBitangents),
        std::back_inserter(bitangents),
        [](const XMFLOAT3& v) { return Vec3D{ v.x, v.y, v.z }; });

    return hr;
}


HRESULT ComputeTangentFrameImpl(
    const std::vector<uint32_t>& indices, 
    size_t nFaces,
    const std::vector<XMFLOAT3>& positions,
    const std::vector<XMFLOAT3>& normals,
    const std::vector<XMFLOAT2>& texcoords,
    size_t nVerts,
    std::vector<XMFLOAT3>& tangents3,
    std::vector<XMFLOAT3>& bitangents)
{
    if (nVerts >= uint32_t(-1))
        return E_INVALIDARG;

    if ((uint64_t(nFaces) * 3) >= UINT32_MAX)
        return HRESULT_E_ARITHMETIC_OVERFLOW;

    static constexpr float EPSILON = 0.0001f;
    static const XMVECTORF32 s_flips = { { { 1.f, -1.f, -1.f, 1.f } } };

    std::vector<XMVECTOR> tangent1(nVerts);
    std::vector<XMVECTOR> tangent2(nVerts);

    for (size_t face = 0; face < nFaces; ++face)
    {
        uint32_t i0 = indices[face * 3];
        uint32_t i1 = indices[face * 3 + 1];
        uint32_t i2 = indices[face * 3 + 2];

        if (i0 == uint32_t(-1)
            || i1 == uint32_t(-1)
            || i2 == uint32_t(-1))
            continue;

        if (i0 >= nVerts
            || i1 >= nVerts
            || i2 >= nVerts)
            return E_UNEXPECTED;

        const XMVECTOR t0 = XMLoadFloat2(&texcoords[i0]);
        const XMVECTOR t1 = XMLoadFloat2(&texcoords[i1]);
        const XMVECTOR t2 = XMLoadFloat2(&texcoords[i2]);

        XMVECTOR s = XMVectorMergeXY(XMVectorSubtract(t1, t0), XMVectorSubtract(t2, t0));

        XMFLOAT4A tmp;
        XMStoreFloat4A(&tmp, s);

        float d = tmp.x * tmp.w - tmp.z * tmp.y;
        d = (fabsf(d) <= EPSILON) ? 1.f : (1.f / d);
        s = XMVectorScale(s, d);
        s = XMVectorMultiply(s, s_flips);

        XMMATRIX m0;
        m0.r[0] = XMVectorPermute<3, 2, 6, 7>(s, g_XMZero);
        m0.r[1] = XMVectorPermute<1, 0, 4, 5>(s, g_XMZero);
        m0.r[2] = m0.r[3] = g_XMZero;

        const XMVECTOR p0 = XMLoadFloat3(&positions[i0]);
        const XMVECTOR p1 = XMLoadFloat3(&positions[i1]);
        XMVECTOR p2 = XMLoadFloat3(&positions[i2]);

        XMMATRIX m1;
        m1.r[0] = XMVectorSubtract(p1, p0);
        m1.r[1] = XMVectorSubtract(p2, p0);
        m1.r[2] = m1.r[3] = g_XMZero;

        const XMMATRIX uv = XMMatrixMultiply(m0, m1);

        tangent1[i0] = XMVectorAdd(tangent1[i0], uv.r[0]);
        tangent1[i1] = XMVectorAdd(tangent1[i1], uv.r[0]);
        tangent1[i2] = XMVectorAdd(tangent1[i2], uv.r[0]);

        tangent2[i0] = XMVectorAdd(tangent2[i0], uv.r[1]);
        tangent2[i1] = XMVectorAdd(tangent2[i1], uv.r[1]);
        tangent2[i2] = XMVectorAdd(tangent2[i2], uv.r[1]);
    }

    for (size_t j = 0; j < nVerts; ++j)
    {
        // Gram-Schmidt orthonormalization
        XMVECTOR b0 = XMLoadFloat3(&normals[j]);
        b0 = XMVector3Normalize(b0);

        const XMVECTOR tan1 = tangent1[j];
        XMVECTOR b1 = XMVectorSubtract(tan1, XMVectorMultiply(XMVector3Dot(b0, tan1), b0));
        b1 = XMVector3Normalize(b1);

        const XMVECTOR tan2 = tangent2[j];
        XMVECTOR b2 = XMVectorSubtract(XMVectorSubtract(tan2, XMVectorMultiply(XMVector3Dot(b0, tan2), b0)), XMVectorMultiply(XMVector3Dot(b1, tan2), b1));
        b2 = XMVector3Normalize(b2);

        // handle degenerate vectors
        const float len1 = XMVectorGetX(XMVector3Length(b1));
        const float len2 = XMVectorGetY(XMVector3Length(b2));

        if ((len1 <= EPSILON) || (len2 <= EPSILON))
        {
            if (len1 > 0.5f)
            {
                // Reset bi-tangent from tangent and normal
                b2 = XMVector3Cross(b0, b1);
            }
            else if (len2 > 0.5f)
            {
                // Reset tangent from bi-tangent and normal
                b1 = XMVector3Cross(b2, b0);
            }
            else
            {
                // Reset both tangent and bi-tangent from normal
                XMVECTOR axis;

                const float d0 = fabsf(XMVectorGetX(XMVector3Dot(g_XMIdentityR0, b0)));
                const float d1 = fabsf(XMVectorGetX(XMVector3Dot(g_XMIdentityR1, b0)));
                const float d2 = fabsf(XMVectorGetX(XMVector3Dot(g_XMIdentityR2, b0)));
                if (d0 < d1)
                {
                    axis = (d0 < d2) ? g_XMIdentityR0 : g_XMIdentityR2;
                }
                else if (d1 < d2)
                {
                    axis = g_XMIdentityR1;
                }
                else
                {
                    axis = g_XMIdentityR2;
                }

                b1 = XMVector3Cross(b0, axis);
                b2 = XMVector3Cross(b0, b1);
            }
        }

        XMStoreFloat3(&tangents3[j], b1);

        XMStoreFloat3(&bitangents[j], b2);

    }

    return S_OK;
}
