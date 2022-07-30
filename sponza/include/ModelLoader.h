#pragma once

#include <assimp/material.h>

#include <assimp/Importer.hpp>
#include <string>
#include <vector>

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
              TextureStorageType storageType)
      : Path(path),
        Type(type),
        StorageType(storageType) {
  }
  TextureInfo()
      : TextureInfo("", TextureType::None, TextureStorageType::None) {
  }
  std::string Path;
  TextureType Type;
  TextureStorageType StorageType;
};

struct Mesh {
  Mesh() {
  }
  Mesh(std::vector<Vertex> vertices,
       std::vector<unsigned int> indices,
       std::vector<TextureInfo> textures,
       std::string name)
      : Vertices(vertices),
        Indices(indices),
        Textures(textures),
        Name(name) {
  }
  std::vector<Vertex> Vertices;
  std::vector<unsigned int> Indices;
  std::vector<TextureInfo> Textures;
  std::string Name;
};

class ModelLoader {
 public:
  std::vector<Mesh> Load(const std::string &path);

 private:
  Assimp::Importer m_importer;
  std::string m_cwd;
};