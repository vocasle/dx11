#pragma once

#include <assimp/material.h>
#include <assimp/texture.h>
#include <d3d11.h>

#include <assimp/Importer.hpp>
#include <string>
#include <vector>

#include "Buffer.h"
#include "NE_Math.h"

struct Vertex {
    Vec4D Position;
    Vec4D Normal;
};

enum class TextureStorageType {
    Embedded,
    Detached,
    None,
};

std::string TextureStorageTypeToStr(TextureStorageType tst);

enum class TextureType {
    Ambient,
    Emissive,
    Diffuse,
    Specular,
    Shininess,
    Normal,
    None,
};

TextureType TextureTypeFromAssimpTextureType(aiTextureType type);
std::string TextureTypeToStr(TextureType tt);

struct TextureInfo {
    TextureInfo(const std::string &path,
                TextureType type,
                TextureStorageType storageType,
                const aiTexture *texturePtr)
        : Path(path),
          Type(type),
          StorageType(storageType),
          TexturePtr(texturePtr) {
    }
    TextureInfo(const std::string &path,
                TextureType type,
                TextureStorageType storageType)
        : TextureInfo(path, type, storageType, nullptr) {
    }
    TextureInfo()
        : TextureInfo("", TextureType::None, TextureStorageType::None) {
    }
    std::string Path;
    TextureType Type;
    TextureStorageType StorageType;
    const aiTexture *TexturePtr;
};

class Mesh {
public:
    Mesh()
        : Mesh({}, {}, {}, "") {
    }
    Mesh(std::vector<Vertex> vertices,
         std::vector<unsigned int> indices,
         std::vector<TextureInfo> textures,
         std::string name)
        : m_vertexBuffer(vertices,
                         BufferDescription(D3D11_USAGE_IMMUTABLE,
                                           D3D11_BIND_VERTEX_BUFFER)),
          m_indexBuffer(indices,
                        BufferDescription(D3D11_USAGE_IMMUTABLE,
                                          D3D11_BIND_INDEX_BUFFER)),
          m_textures(textures),
          m_name(name) {
    }

    void
    CreateDeviceDependentResources(ID3D11Device *device) {
        CreateBuffers(device);
    }

    [[nodiscard]] ID3D11Buffer *
    GetVertexBuffer() const {
        return m_vertexBuffer.GetBuffer();
    }

    [[nodiscard]] ID3D11Buffer *
    GetIndexBuffer() const {
        return m_indexBuffer.GetBuffer();
    }

    [[nodiscard]] unsigned int
    GetIndexCount() const {
        return m_indexBuffer.Count();
    }

    [[nodiscard]] unsigned int
    GetVertexSize() const {
        return m_vertexBuffer.GetValueSize();
    }

    [[nodiscard]] const std::vector<TextureInfo> &
    GetTextures() const {
        return m_textures;
    }

private:
    void
    CreateBuffers(ID3D11Device *device) {
        m_vertexBuffer.Create(device);
        m_indexBuffer.Create(device);
    }

    Buffer<Vertex> m_vertexBuffer;
    Buffer<unsigned int> m_indexBuffer;
    std::vector<TextureInfo> m_textures;
    std::string m_name;
};

class ModelLoader {
public:
    std::vector<Mesh> Load(const std::string &path);
    void FreeScene();
    const aiScene *
    GetScenePtr() const {
        return m_scene;
    }

private:
    Assimp::Importer m_importer;
    std::string m_cwd;
    const aiScene *m_scene;
};