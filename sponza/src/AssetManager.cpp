#include "AssetManager.h"

#include <d3d11.h>

AssetManager::AssetManager(DeviceResources &deviceResources)
    : m_deviceResources(&deviceResources) {
}

Texture*
AssetManager::LoadTexture(const std::string &path) {
    if (Texture* tex = GetTexture(path))
        return tex;

    m_textures.insert(std::make_pair(path, Texture(path, m_deviceResources->GetDevice())));
    return &m_textures.at(path);
}

Texture *
AssetManager::GetTexture(const std::string &path) {
    auto it = m_textures.find(path);
    if (it != std::end(m_textures))
        return &it->second;
    return nullptr;
}
