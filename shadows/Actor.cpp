#include "Actor.h"
#include "MeshGenerator.h"
#include "Utils.h"
#include "stb_image.h"

Actor::Actor ()
    : m_IndexBuffer{ nullptr }, m_VertexBuffer{ nullptr },
      m_Vertices{}, m_Indices{}, m_World{ MathMat4X4Identity () },
      m_DiffuseTexture{ nullptr }, m_SpecularTexture{ nullptr },
      m_GlossTexture{ nullptr }, m_NormalTexture{ nullptr }, m_Material{}
{
}

Actor::Actor (const Actor &actor)
{
  m_IndexBuffer = actor.m_IndexBuffer;
  m_VertexBuffer = actor.m_VertexBuffer;
  m_Vertices = actor.m_Vertices;
  m_Indices = actor.m_Indices;
  m_World = actor.m_World;
  m_DiffuseTexture = actor.m_DiffuseTexture;
  m_SpecularTexture = actor.m_SpecularTexture;
  m_GlossTexture = actor.m_GlossTexture;
  m_NormalTexture = actor.m_NormalTexture;
  m_Material = actor.m_Material;
}

Actor &
Actor::operator= (const Actor &actor)
{
  if (this != &actor)
    {
      Actor temp (actor);
      temp.Swap (*this);
    }
  return *this;
}

Actor::Actor (Actor &&actor) noexcept { actor.Swap (*this); }

Actor &
Actor::operator= (Actor &&actor) noexcept
{
  Actor temp (std::move (actor));
  temp.Swap (*this);
  return *this;
}

Actor::Actor (Mesh *mesh) : Actor () { LoadMesh (mesh); }

Actor::~Actor () {}

ID3D11ShaderResourceView **
Actor::GetShaderResources () const
{
  static ID3D11ShaderResourceView *shaderResources[4] = {};
  shaderResources[0] = m_DiffuseTexture.Get ();
  shaderResources[1] = m_SpecularTexture.Get ();
  shaderResources[2] = m_GlossTexture.Get ();
  shaderResources[3] = m_NormalTexture.Get ();
  return shaderResources;
}

void
Actor::Swap (Actor &actor)
{
  std::swap (m_World, actor.m_World);
  std::swap (m_Vertices, actor.m_Vertices);
  std::swap (m_Indices, actor.m_Indices);
  std::swap (m_Material, actor.m_Material);
  std::swap (m_DiffuseTexture, actor.m_DiffuseTexture);
  std::swap (m_SpecularTexture, actor.m_SpecularTexture);
  std::swap (m_GlossTexture, actor.m_GlossTexture);
  std::swap (m_NormalTexture, actor.m_NormalTexture);
  std::swap (m_IndexBuffer, actor.m_IndexBuffer);
  std::swap (m_VertexBuffer, actor.m_VertexBuffer);
}

void
Actor::LoadMesh (Mesh *mesh)
{
  m_Vertices.reserve (mesh->Faces.size () + m_Vertices.size ());
  m_Indices.reserve (mesh->Faces.size () + m_Indices.size ());

  Vertex vert = {};

  for (uint32_t j = 0; j < mesh->Faces.size (); ++j)
    {
      const Face &face = mesh->Faces[j];
      const Position &pos = mesh->Positions[face.posIdx];
      const Normal &norm = mesh->Normals[face.normIdx];
      const TexCoord &tc = mesh->TexCoords[face.texIdx];

      vert.Position.X = pos.x;
      vert.Position.Y = pos.y;
      vert.Position.Z = pos.z;

      vert.Normal.X = norm.x;
      vert.Normal.Y = norm.y;
      vert.Normal.Z = norm.z;

      vert.TexCoords.X = tc.u;
      vert.TexCoords.Y = tc.v;

      assert (m_Indices.size () + 1 <= mesh->Faces.size ());
      m_Indices.emplace_back (m_Indices.size ());
      assert (m_Vertices.size () + 1 <= mesh->Faces.size ());
      m_Vertices.emplace_back (vert);
    }
}

void
Actor::GenerateTangents ()
{
  ComputeTangentFrame (m_Indices, m_Vertices);
}

void
Actor::LoadModel (const char *filename)
{
  std::unique_ptr<Model> model = OLLoad (filename);
  if (!model)
    {
      UTILS_FATAL_ERROR ("Failed to load model %s", filename);
    }

  size_t numFaces = 0;
  for (uint32_t i = 0; i < model->Meshes.size (); ++i)
    {
      const Mesh *mesh = &model->Meshes[i];
      numFaces += mesh->Faces.size ();
    }

  m_Vertices.reserve (numFaces);
  m_Indices.reserve (numFaces);

  size_t posOffs = 0;
  size_t normOffs = 0;
  size_t tcOffs = 0;
  Vertex vert = {};

  for (uint32_t i = 0; i < model->Meshes.size (); ++i)
    {
      const Mesh *mesh = &model->Meshes[i];
      for (uint32_t j = 0; j < mesh->Faces.size (); ++j)
        {
          const Face &face = model->Meshes[i].Faces[j];
          const Position &pos = mesh->Positions[face.posIdx - posOffs];
          const Normal &norm = mesh->Normals[face.normIdx - normOffs];
          const TexCoord &tc = mesh->TexCoords[face.texIdx - tcOffs];

          vert.Position.X = pos.x;
          vert.Position.Y = pos.y;
          vert.Position.Z = pos.z;

          vert.Normal.X = norm.x;
          vert.Normal.Y = norm.y;
          vert.Normal.Z = norm.z;

          vert.TexCoords.X = tc.u;
          vert.TexCoords.Y = tc.v;

          m_Indices.emplace_back (m_Indices.size ());
          m_Vertices.emplace_back (vert);
        }
      posOffs += mesh->Positions.size ();
      normOffs += mesh->Normals.size ();
      tcOffs += mesh->TexCoords.size ();
    }

  GenerateTangents ();
}

void
Actor::CreateVertexBuffer (ID3D11Device *device)
{
  D3D11_SUBRESOURCE_DATA subresourceData = {};
  subresourceData.pSysMem = &m_Vertices[0];

  D3D11_BUFFER_DESC bufferDesc = {};
  bufferDesc.ByteWidth = sizeof (Vertex) * m_Vertices.size ();
  bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  bufferDesc.StructureByteStride = sizeof (Vertex);
  bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;

  HR (device->CreateBuffer (&bufferDesc, &subresourceData, &m_VertexBuffer))
}

void
Actor::CreateIndexBuffer (ID3D11Device *device)
{
  D3D11_SUBRESOURCE_DATA subresourceData = {};
  subresourceData.pSysMem = &m_Indices[0];

  D3D11_BUFFER_DESC bufferDesc = {};
  bufferDesc.ByteWidth = sizeof (uint32_t) * m_Indices.size ();
  bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
  bufferDesc.StructureByteStride = 0;
  bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;

  HR (device->CreateBuffer (&bufferDesc, &subresourceData, &m_IndexBuffer));
}

void
Actor::Translate (const Vec3D offset)
{
  const Mat4X4 offsetMat = MathMat4X4TranslateFromVec3D (&offset);
  m_World = MathMat4X4MultMat4X4ByMat4X4 (&m_World, &offsetMat);
}

void
Actor::Rotate (const float pitch, const float yaw, const float roll)
{
  const Vec3D angles = { pitch, yaw, roll };
  const Mat4X4 rotMat = MathMat4X4RotateFromVec3D (&angles);
  m_World = MathMat4X4MultMat4X4ByMat4X4 (&m_World, &rotMat);
}

void
Actor::Scale (const float s)
{
  const Vec3D scale = { s, s, s };
  const Mat4X4 scaleMat = MathMat4X4ScaleFromVec3D (&scale);
  m_World = MathMat4X4MultMat4X4ByMat4X4 (&m_World, &scaleMat);
}

void
Actor::LoadTexture (const char *filename, enum TextureType type,
                    ID3D11Device *device, ID3D11DeviceContext *context)
{
  int width = 0;
  int height = 0;
  int channelsInFile = 0;
  const int desiredChannels = 4;

  unsigned char *bytes = stbi_load (filename, &width, &height, &channelsInFile,
                                    desiredChannels);
  if (!bytes)
    {
      UtilsDebugPrint ("ERROR: Failed to load texture from %s\n", filename);
      ExitProcess (EXIT_FAILURE);
    }
  Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
  {
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

    D3D11_SUBRESOURCE_DATA subresourceData = {};
    subresourceData.pSysMem = bytes;
    subresourceData.SysMemPitch
        = width * sizeof (unsigned char) * desiredChannels;

    HR (device->CreateTexture2D (&desc, &subresourceData,
                                 texture.ReleaseAndGetAddressOf ()))
  }

  {
    // TODO: Fix mip map generation
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    memset (&srvDesc, 0, sizeof (srvDesc));
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = -1;

    switch (type)
      {
      case TextureType::Diffuse:
        HR (device->CreateShaderResourceView (
            texture.Get (), &srvDesc,
            m_DiffuseTexture.ReleaseAndGetAddressOf ()))
        context->GenerateMips (m_DiffuseTexture.Get ());
        break;
      case TextureType::Specular:
        HR (device->CreateShaderResourceView (
            texture.Get (), &srvDesc,
            m_SpecularTexture.ReleaseAndGetAddressOf ()));
        context->GenerateMips (m_SpecularTexture.Get ());
        break;
      case TextureType::Gloss:
        HR (device->CreateShaderResourceView (
            texture.Get (), &srvDesc,
            m_GlossTexture.ReleaseAndGetAddressOf ()));
        context->GenerateMips (m_GlossTexture.Get ());
        break;
      case TextureType::Normal:
        HR (device->CreateShaderResourceView (
            texture.Get (), &srvDesc,
            m_NormalTexture.ReleaseAndGetAddressOf ()));
        context->GenerateMips (m_NormalTexture.Get ());
        break;
      default:
        break;
      }
  }

  stbi_image_free (bytes);
}

void
Actor::SetMaterial (const Material *material)
{
  m_Material = *material;
}