#pragma once

#include "Texture.h"
#include "DeviceResources.h"

#include <string>
#include <unordered_map>
#include "ModelLoader.h"

class AssetManager
{
public:
    AssetManager(DeviceResources& deviceResources);
    Texture* LoadTexture(const std::string& path);
    Texture* GetTexture(const std::string& path);
    std::vector<Mesh> LoadModel(const std::string& path);
    Texture* LoadEmbeddedTexture(const std::string& path);

private:

    DeviceResources* m_deviceResources;
    std::unordered_map<std::string, Texture> m_textures;
    ModelLoader m_modelLoader;
};