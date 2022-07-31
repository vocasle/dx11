#include "ModelLoader.h"

#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "Utils.h"

static void ProcessNode(aiNode *node,
                        const aiScene *scene,
                        std::vector<Mesh> &meshes);

static Mesh ProcessMesh(aiMesh *mesh, const aiScene *scene);

std::vector<Mesh>
ModelLoader::Load(const std::string &path) {
    m_scene = m_importer.ReadFile(
        path, aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);

    if (m_scene == nullptr)
        return {};

    m_cwd = path.substr(0, path.find_last_of("/\\"));

    std::vector<Mesh> meshes;

    ProcessNode(m_scene->mRootNode, m_scene, meshes);

    m_cwd = "";

    return meshes;
}
void
ModelLoader::FreeScene() {
    m_importer.FreeScene();
}

static void
ProcessNode(aiNode *node, const aiScene *scene, std::vector<Mesh> &meshes) {
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(ProcessMesh(mesh, scene));
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        ProcessNode(node->mChildren[i], scene, meshes);
    }
}

static Mesh
ProcessMesh(aiMesh *mesh, const aiScene *scene) {
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
        for (const aiTextureType &type : supportedTypes) {
            if (material->GetTextureCount(type) > 0) {
                material->GetTexture(type, 0, &texturePath);

                const aiTexture *tex =
                    scene->GetEmbeddedTexture(texturePath.C_Str());

                textures.emplace_back(texturePath.C_Str(),
                                      TextureTypeFromAssimpTextureType(type),
                                      tex ? TextureStorageType::Embedded
                                          : TextureStorageType::Detached,
                                      tex);
            }
        }
    }

    return {vertices, indices, textures, mesh->mName.C_Str()};
}

TextureType
TextureTypeFromAssimpTextureType(aiTextureType type) {
    switch (type) {
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

std::string
TextureTypeToStr(TextureType tt) {
    switch (tt) {
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

std::string
TextureStorageTypeToStr(TextureStorageType tst) {
    switch (tst) {
        case TextureStorageType::Detached:
            return "Detached";
        case TextureStorageType::Embedded:
            return "Embedded";
        default:
            return "None";
    }
}