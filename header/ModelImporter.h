#pragma once

#include <vector>
#include <string>
#include <include/glm/glm.hpp>
#include <include/glm/gtc/matrix_transform.hpp>
#include <include/glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include "TextureManager.h"
#include "assimp/matrix4x4.h"
#include "assimp/scene.h"

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    GLuint VAO, VBO, EBO;
    MaterialTextures materialTextures;
    glm::mat4 transform = glm::mat4(1.0f);

    void setupMesh();
    void Draw() const;
};

class ModelImporter {
public:
    ModelImporter() = default;

    bool loadModel(const std::string& path);
    const std::vector<Mesh>& getMeshes() const { return meshes; }
    void centerAndScaleModel(float scaleTo = 1.0f);
    glm::vec3 getBoundingBoxCenter() const;
    float getBoundBoxRadius() const;


private:
    std::vector<Mesh> meshes;
    std::string directory;

    void processNode(aiNode* node, const aiScene* scene, const aiMatrix4x4& parentTransform = aiMatrix4x4());
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
};
