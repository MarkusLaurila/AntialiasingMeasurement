#pragma once

#include <vector>
#include <string>
#include <include/glm/glm.hpp>
#include <GL/glew.h>
#include "TextureManager.h"

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    GLuint VAO, VBO, EBO;
    size_t indexCount;
    MaterialTextures materialTextures;

    void setupMesh();
    void Draw() const;
};

class ModelImporter {
public:
    ModelImporter() = default;

    bool loadModel(const std::string& path);
    const std::vector<Mesh>& getMeshes() const;

private:
    std::vector<Mesh> meshes;
    std::string directory;

    void processNode(class aiNode* node, const class aiScene* scene);
    Mesh processMesh(class aiMesh* mesh, const class aiScene* scene);
};
