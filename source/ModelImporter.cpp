#include "ModelImporter.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <iostream>
#include <include/glm/gtc/matrix_transform.hpp>
#include <include/glm/gtc/type_ptr.hpp>
#include <include/glm/glm.hpp>
#include <cfloat>

template<typename T>
T maxComponent(const glm::tvec3<T>& v) {
    return glm::max(glm::max(v.x, v.y), v.z);
}

void Mesh::setupMesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

    glBindVertexArray(0);
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

    if (!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
        std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return false;
    }

    directory = path.substr(0, path.find_last_of('/'));
    meshes.clear();
    processNode(scene->mRootNode, scene, aiMatrix4x4());
    return true;
}

void ModelImporter::processNode(aiNode* node, const aiScene* scene, const aiMatrix4x4& parentTransform) {
    aiMatrix4x4 globalTransform = parentTransform * node->mTransformation;

    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        Mesh m = processMesh(mesh, scene);
        m.transform = glm::transpose(glm::make_mat4(&globalTransform.a1));
        meshes.push_back(m);
    }

    for (unsigned int i = 0; i < node->mNumChildren; ++i)
        processNode(node->mChildren[i], scene, globalTransform);
}

Mesh ModelImporter::processMesh(aiMesh* mesh, const aiScene* scene) {
    std::vector<Vertex> vertices(mesh->mNumVertices);
    std::vector<unsigned int> indices;

    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        Vertex vertex;
        vertex.Position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
        vertex.Normal = mesh->HasNormals() ? glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z) : glm::vec3(0.0f);
        vertex.TexCoords = mesh->mTextureCoords[0] ? glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y) : glm::vec2(0.0f);
        vertices[i] = vertex;
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; ++j)
            indices.push_back(face.mIndices[j]);
    }

    Mesh m;
    m.vertices = std::move(vertices);
    m.indices = std::move(indices);

    std::string baseColor, normal, roughness, ao, mask, unpacked,metallic,displacement;

    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        auto loadTexture = [&](aiTextureType type) -> std::string {
            if (material->GetTextureCount(type) > 0) {
                aiString str;
                if (material->GetTexture(type, 0, &str) == AI_SUCCESS)
                    return directory + "/" + std::string(str.C_Str());
            }
            return "";
        };

        baseColor = loadTexture(aiTextureType_DIFFUSE);
        normal = loadTexture(aiTextureType_NORMALS);
        if (normal.empty()) normal = loadTexture(aiTextureType_HEIGHT);
        roughness = loadTexture(aiTextureType_SPECULAR);
        ao = loadTexture(aiTextureType_AMBIENT);
        metallic = loadTexture(aiTextureType_METALNESS);
        displacement = loadTexture(aiTextureType_DISPLACEMENT);


        aiString name;
        material->Get(AI_MATKEY_NAME, name);
        std::string matName(name.C_Str());
        if (baseColor.empty()) {
            if (matName.find("Sand beach") != std::string::npos) {
                baseColor = Textures::BaseColors::SAND_BEACH_BC;
                normal = Textures::Normals::SAND_BEACH_NORMAL;
                roughness = Textures::Roughness::SAND_BEACH_ROUGHNESS;
                ao = Textures::AmbientOcclusion::SAND_ALPHA;
                metallic = Textures::Metallic::SAND_METALLIC;
                displacement = Textures::Displacement::SAND_DISPLACEMENT;

            }
            else if (matName.find("Tower") != std::string::npos) {
                baseColor = Textures::BaseColors::TOWER_BC;
                normal = Textures::Normals::TOWER_NORMAL;
                roughness = Textures::Roughness::TOWER_ROUGHNESS;
                ao = "";
            }
            else if (matName.find("Door") != std::string::npos) {
                baseColor = Textures::BaseColors::WOOD_DOOR_BC;
                normal = Textures::Normals::WOOD_DOOR_NORMAL;
                roughness = Textures::Roughness::WOOD_DOOR_ROUGHNESS;
                ao = "";
            }
            else if (matName.find("M Grass Generic Set") != std::string::npos) {
                baseColor = Textures::BaseColors::GRASS_BC;
                normal = Textures::Normals::GRASS_NORMAL;
                roughness = Textures::Roughness::GRASS_ROUGHNESS;
                ao = Textures::AmbientOcclusion::GRASS_AO;
                mask = Textures::Mask::GRASS_MASK;
            }
            else if (matName.find("Material") != std::string::npos) {
                baseColor = Textures::BaseColors::TOWER_BC;
                normal = Textures::Normals::TOWER_NORMAL;
                roughness = Textures::Roughness::TOWER_ROUGHNESS;
            }
            else if (matName.find("Fence") != std::string::npos) {
                baseColor = Textures::BaseColors::WOODEN_BOARD_BC;
                normal = Textures::Normals::WOODEN_BOARD_NORMAL;
                roughness = Textures::Roughness::WOODEN_BOARD_ROUGHNESS;
            }
            else if (matName.find("Material.001") != std::string::npos) {
                baseColor = Textures::BaseColors::TOWER_BC;
                normal = Textures::Normals::TOWER_NORMAL;
                roughness = Textures::Roughness::TOWER_ROUGHNESS;
            }
            // Add more fallback mappings here
        }
    }
    m.materialTextures = TextureManager::LoadMaterialTextures(baseColor, normal, roughness, ao, mask, unpacked,metallic, displacement);
    // std::cout << "Mesh loaded with textures: "
    //       << "\n  BaseColor: " << baseColor
    //       << "\n  Normal:    " << normal
    //       << "\n  Roughness: " << roughness
    //       << "\n  AO:        " << ao
    //       << "\n  OpenGL IDs:"
    //       << "\n    BaseColorID: " << m.materialTextures.baseColorID
    //       << "\n    NormalID:    " << m.materialTextures.normalID
    //       << "\n    RoughnessID: " << m.materialTextures.roughnessID
    //       << "\n    AO_ID:       " << m.materialTextures.aoID
    //       << std::endl;


    m.setupMesh();
    return m;
}






void ModelImporter::centerAndScaleModel(float scaleTo) {
    if (meshes.empty()) return;

    glm::vec3 min(FLT_MAX), max(-FLT_MAX);

    for (const auto& mesh : meshes) {
        for (const auto& vertex : mesh.vertices) {
            glm::vec4 pos = mesh.transform * glm::vec4(vertex.Position, 1.0f);
            min = glm::min(min, glm::vec3(pos));
            max = glm::max(max, glm::vec3(pos));
        }
    }

    glm::vec3 center = (min + max) * 0.5f;
    float maxExtent = maxComponent(max - min);
    float scaleFactor = scaleTo / maxExtent;

    for (auto& mesh : meshes) {
        mesh.transform = glm::translate(glm::mat4(1.0f), -center) * mesh.transform;
        mesh.transform = glm::scale(glm::mat4(1.0f), glm::vec3(scaleFactor)) * mesh.transform;
    }
}

glm::vec3 ModelImporter::getBoundingBoxCenter() const {
    if (meshes.empty()) return glm::vec3(0.0f);

    glm::vec3 min(FLT_MAX), max(-FLT_MAX);

    for (const auto& mesh : meshes) {
        for (const auto& vertex : mesh.vertices) {
            glm::vec4 pos = mesh.transform * glm::vec4(vertex.Position, 1.0f);
            min = glm::min(min, glm::vec3(pos));
            max = glm::max(max, glm::vec3(pos));
        }
    }

    return (min + max) * 0.5f;
}

float ModelImporter::getBoundBoxRadius() const {
    if (meshes.empty()) return 0.0f;

    glm::vec3 center = getBoundingBoxCenter();
    float radius = 0.0f;

    for (const auto& mesh : meshes) {
        for (const auto& vertex : mesh.vertices) {
            glm::vec4 pos = mesh.transform * glm::vec4(vertex.Position, 1.0f);
            float dist = glm::length(glm::vec3(pos) - center);
            if (dist > radius) radius = dist;
        }
    }

    return radius;
}
