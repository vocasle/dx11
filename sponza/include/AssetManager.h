#pragma once

#include "Texture.h"
#include "DeviceResources.h"

#include <string>
#include <unordered_map>

class AssetManager
{
public:
    AssetManager(DeviceResources& deviceResources);
    void LoadTexture(const std::string& path);
    Texture* GetTexture(const std::string& path);

private:

    DeviceResources* m_deviceResources;
    std::unordered_map<std::string, Texture> m_textures;
};