#include "ModelLoader.h"
#include "NE_Math.h"
#include "Utils.h"

#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <vector>

struct Vertex
{
    Vec4D Position;
    Vec4D Normal;
};

enum class TextureStorageType
{
    Embedded,
    Detached,
    None,
};

static std::string TextureStorageTypeToStr(TextureStorageType tst)
{
    switch (tst)
    {
    case TextureStorageType::Detached:
        return "Detached";
    case TextureStorageType::Embedded:
        return "Embedded";
    default:
        return "None";
    }
}

enum class TextureType
{
    Ambient,
    Emissive,
    Diffuse,
    Specular,
    Shininess,
    Normal,
    None,
};

TextureType TextureTypeFromAssimpTextureType(aiTextureType type)
{
    switch(type)
    {
    case aiTextureType_AMBIENT:
        return TextureType::Ambient;
    case aiTextureType_DIFFUSE:
        return TextureType::Diffuse;
    case aiTextureType_SPECULAR:
        return TextureType::Specular;
    case aiTextureType_NORMALS:
        return TextureType::Normal;
    case aiTextureType_SHININESS:
        return TextureType::Shininess;
    case aiTextureType_EMISSIVE:
        return TextureType::Emissive;
    default:
        return TextureType::None;
    }
}

static std::string TextureTypeToStr(TextureType tt)
{
    switch (tt)
    {
    case TextureType::Ambient:
        return "Ambient";
    case TextureType::Emissive:
        return "Emissive";
    case TextureType::Diffuse:
        return "Diffuse";
    case TextureType::Specular:
        return "Specular";
    case TextureType::Shininess:
        return "Shininess";
    case TextureType::Normal:
        return "Normal";
    default:
        return "None";
    }
}

struct TextureInfo
{
    TextureInfo(const std::string& path, TextureType type, TextureStorageType storageType): Path(path), Type(type), StorageType(storageType) {}
    TextureInfo(): TextureInfo("", TextureType::None, TextureStorageType::None) {}
    std::string Path;
    TextureType Type;
    TextureStorageType StorageType;
};

struct Mesh
{
    Mesh() {}
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<TextureInfo> textures, std::string name):
        Vertices(vertices),
        Indices(indices),
        Textures(textures),
        Name(name)
        {}
    std::vector<Vertex> Vertices;
    std::vector<unsigned int> Indices;
    std::vector<TextureInfo> Textures;
    std::string Name;
};

static void ProcessNode(aiNode *node, const aiScene *scene, std::vector<Mesh>& meshes);
static Mesh ProcessMesh(aiMesh *mesh, const aiScene *scene);
void ModelLoader::Load(const std::string &path)
{
	const aiScene *pScene = m_importer.ReadFile(
		path, aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);

	if (pScene == nullptr)
		return;

	m_cwd = path.substr(0, path.find_last_of("/\\"));

    std::vector<Mesh> meshes;

	ProcessNode(pScene->mRootNode, pScene, meshes);

    for (const Mesh& m : meshes)
    {
        UtilsDebugPrint("Mesh: %s, num vertices: %lu, num indices: %lu, num textures: %lu\n", m.Name.c_str(), m.Vertices.size(), m.Indices.size(), m.Textures.size());
        for (const TextureInfo& info : m.Textures)
        {
            UtilsDebugPrint("Mesh: %s, texture type: %s, texture path: %s, texture storage type: %s\n", 
                m.Name.c_str(),
                TextureTypeToStr(info.Type).c_str(),
                info.Path.c_str(),
                TextureStorageTypeToStr(info.StorageType).c_str());
        }
    }

	m_cwd = "";
}

static void ProcessNode(aiNode *node, const aiScene *scene, std::vector<Mesh>& meshes)
{
	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(ProcessMesh(mesh, scene));
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		ProcessNode(node->mChildren[i], scene, meshes);
	}
}

static Mesh ProcessMesh(aiMesh *mesh, const aiScene *scene)
{
	// Data to fill
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<TextureInfo> textures;

	// Walk through each of the mesh's vertices
	for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
		Vertex vertex;

		vertex.Position.X = mesh->mVertices[i].x;
		vertex.Position.Y = mesh->mVertices[i].y;
		vertex.Position.Z = mesh->mVertices[i].z;

        if (mesh->HasNormals()) {
            vertex.Normal.X = mesh->mNormals[i].x;
            vertex.Normal.Y = mesh->mNormals[i].y;
            vertex.Normal.Z = mesh->mNormals[i].z;
        }

		if (mesh->mTextureCoords[0]) {
			vertex.Position.W = (float)mesh->mTextureCoords[0][i].x;
			vertex.Normal.W = (float)mesh->mTextureCoords[0][i].y;
		}

		vertices.push_back(vertex);
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];

		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	if (mesh->mMaterialIndex >= 0) {
		aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

        static const aiTextureType supportedTypes[] = {
            aiTextureType_AMBIENT,
            aiTextureType_DIFFUSE,
            aiTextureType_SPECULAR, 
            aiTextureType_NORMALS,
            aiTextureType_SHININESS,
            aiTextureType_EMISSIVE,
        };

        aiString texturePath;
        for (const aiTextureType& type : supportedTypes) {
            if (material->GetTextureCount(type) > 0) {
                material->GetTexture(type, 0, &texturePath);

                const aiTexture* tex = scene->GetEmbeddedTexture(texturePath.C_Str());
                TextureStorageType texType = TextureStorageType::Detached;
                if (!tex)
                {
                    UtilsDebugPrint("WARN: %s is an embedded texture and should be loaded differently", texturePath.C_Str());
                    texType = TextureStorageType::Embedded;
                }
                textures.emplace_back(texturePath.C_Str(), TextureTypeFromAssimpTextureType(type), texType);
            }
        }
	}

	return Mesh(vertices, indices, textures, mesh->mName.C_Str());
}

// std::vector<Texture> ModelLoader::loadMaterialTextures(aiMaterial * mat, aiTextureType type, std::string typeName, const aiScene * scene) {
// 	std::vector<Texture> textures;
// 	for (UINT i = 0; i < mat->GetTextureCount(type); i++) {
// 		aiString str;
// 		mat->GetTexture(type, i, &str);
// 		// Check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
// 		bool skip = false;
// 		for (UINT j = 0; j < textures_loaded_.size(); j++) {
// 			if (std::strcmp(textures_loaded_[j].path.c_str(), str.C_Str()) == 0) {
// 				textures.push_back(textures_loaded_[j]);
// 				skip = true; // A texture with the same filepath has already been loaded, continue to next one. (optimization)
// 				break;
// 			}
// 		}
// 		if (!skip) {   // If texture hasn't been loaded already, load it
// 			HRESULT hr;
// 			Texture texture;

// 			const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(str.C_Str());
// 			if (embeddedTexture != nullptr) {
// 				texture.texture = loadEmbeddedTexture(embeddedTexture);
// 			} else {
// 				std::string filename = std::string(str.C_Str());
// 				filename = directory_ + '/' + filename;
// 				std::wstring filenamews = std::wstring(filename.begin(), filename.end());
// 				hr = CreateWICTextureFromFile(dev_, devcon_, filenamews.c_str(), nullptr, &texture.texture);
// 				if (FAILED(hr))
// 					MessageBox(hwnd_, "Texture couldn't be loaded", "Error!", MB_ICONERROR | MB_OK);
// 			}
// 			texture.type = typeName;
// 			texture.path = str.C_Str();
// 			textures.push_back(texture);
// 			this->textures_loaded_.push_back(texture);  // Store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
// 		}
// 	}
// 	return textures;
// }

// void ModelLoader::Close() {
// 	for (auto& t : textures_loaded_)
// 		t.Release();

// 	for (size_t i = 0; i < meshes_.size(); i++) {
// 		meshes_[i].Close();
// 	}
// }

// ID3D11ShaderResourceView * ModelLoader::loadEmbeddedTexture(const aiTexture* embeddedTexture) {
// 	HRESULT hr;
// 	ID3D11ShaderResourceView *texture = nullptr;

// 	if (embeddedTexture->mHeight != 0) {
// 		// Load an uncompressed ARGB8888 embedded texture
// 		D3D11_TEXTURE2D_DESC desc;
// 		desc.Width = embeddedTexture->mWidth;
// 		desc.Height = embeddedTexture->mHeight;
// 		desc.MipLevels = 1;
// 		desc.ArraySize = 1;
// 		desc.SampleDesc.Count = 1;
// 		desc.SampleDesc.Quality = 0;
// 		desc.Usage = D3D11_USAGE_DEFAULT;
// 		desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
// 		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
// 		desc.CPUAccessFlags = 0;
// 		desc.MiscFlags = 0;

// 		D3D11_SUBRESOURCE_DATA subresourceData;
// 		subresourceData.pSysMem = embeddedTexture->pcData;
// 		subresourceData.SysMemPitch = embeddedTexture->mWidth * 4;
// 		subresourceData.SysMemSlicePitch = embeddedTexture->mWidth * embeddedTexture->mHeight * 4;

// 		ID3D11Texture2D *texture2D = nullptr;
// 		hr = dev_->CreateTexture2D(&desc, &subresourceData, &texture2D);
// 		if (FAILED(hr))
// 			MessageBox(hwnd_, "CreateTexture2D failed!", "Error!", MB_ICONERROR | MB_OK);

// 		hr = dev_->CreateShaderResourceView(texture2D, nullptr, &texture);
// 		if (FAILED(hr))
// 			MessageBox(hwnd_, "CreateShaderResourceView failed!", "Error!", MB_ICONERROR | MB_OK);

// 		return texture;
// 	}

// 	// mHeight is 0, so try to load a compressed texture of mWidth bytes
// 	const size_t size = embeddedTexture->mWidth;

// 	hr = CreateWICTextureFromMemory(dev_, devcon_, reinterpret_cast<const unsigned char*>(embeddedTexture->pcData), size, nullptr, &texture);
// 	if (FAILED(hr))
// 		MessageBox(hwnd_, "Texture couldn't be created from memory!", "Error!", MB_ICONERROR | MB_OK);

// 	return texture;
// }