#include "ModelImporter.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>

void Mesh::setupMesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Mesh::Draw() const {
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

bool ModelImporter::loadModel(const std::string& path) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path,
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_FlipUVs |
        aiProcess_CalcTangentSpace);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return false;
    }

    directory = path.substr(0, path.find_last_of('/'));
    meshes.clear();
    processNode(scene->mRootNode, scene);
    return true;
}

Mesh ModelImporter::processMesh(aiMesh* mesh, const aiScene* scene) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        Vertex vertex;
        vertex.Position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
        vertex.Normal = mesh->HasNormals() ? glm::vec3{ mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z } : glm::vec3(0.0f);
        vertex.TexCoords = (mesh->mTextureCoords[0] != nullptr)
    ? glm::vec2{ mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y }
        : glm::vec2(0.0f);
        vertices.push_back(vertex);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; ++j) {
            indices.push_back(face.mIndices[j]);
        }
    }

    Mesh m;
    m.vertices = std::move(vertices);
    m.indices = std::move(indices);

    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        auto loadTexture = [&](aiTextureType type) -> std::string {
            if (material->GetTextureCount(type) > 0) {
                aiString str;
                if (material->GetTexture(type, 0, &str) == AI_SUCCESS) {
                    return directory + "/" + std::string(str.C_Str());
                }
            }
            return "";
        };

        std::string baseColorPath  = loadTexture(aiTextureType_DIFFUSE);
        std::string normalPath     = loadTexture(aiTextureType_NORMALS);
        if (normalPath.empty()) normalPath = loadTexture(aiTextureType_HEIGHT);
        std::string roughnessPath  = loadTexture(aiTextureType_SPECULAR);
        std::string aoPath         = loadTexture(aiTextureType_AMBIENT);

        m.materialTextures = TextureManager::LoadMaterialTextures(baseColorPath, normalPath, roughnessPath, aoPath);
    }

    m.setupMesh();
    return m;
}

void ModelImporter::processNode(aiNode* node, const aiScene* scene) {
    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }

    for (unsigned int i = 0; i < node->mNumChildren; ++i) {
        processNode(node->mChildren[i], scene);
    }
}

const std::vector<Mesh>& ModelImporter::getMeshes() const {
    return meshes;
}
