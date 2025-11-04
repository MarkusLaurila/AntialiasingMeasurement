#pragma once

#include <vector>
#include <string>
#include <include/glm/glm.hpp>
#include <GL/glew.h>


// Vertex structure
struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

// Mesh structure
struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    GLuint VAO, VBO, EBO;

    void setupMesh();
    void Draw() const;
};

// Model Importer class
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

