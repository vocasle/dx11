#pragma once

#include <string>

#include <assimp/Importer.hpp>

class ModelLoader
{
    public:
    void Load(const std::string& path);
    private:

    Assimp::Importer m_importer;
    std::string m_cwd;
};

// #include <vector>
// #include <d3d11_1.h>
// #include <DirectXMath.h>




// #include "mesh.h"
// // #include "TextureLoader.h"

// using namespace DirectX;

// class ModelLoader
// {
// public:
// 	ModelLoader();
// 	~ModelLoader();

// 	bool Load(HWND hwnd, ID3D11Device* dev, ID3D11DeviceContext* devcon, std::string filename);
// 	void Draw(ID3D11DeviceContext* devcon);

// 	void Close();
// private:
// 	ID3D11Device *dev_;
// 	ID3D11DeviceContext *devcon_;
// 	std::vector<Mesh> meshes_;
// 	std::string directory_;
// 	std::vector<Texture> textures_loaded_;
// 	HWND hwnd_;

// 	void processNode(aiNode* node, const aiScene* scene);
// 	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
// 	std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName, const aiScene* scene);
// 	ID3D11ShaderResourceView* loadEmbeddedTexture(const aiTexture* embeddedTexture);
// };
