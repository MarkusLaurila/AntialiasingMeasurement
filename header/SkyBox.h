#pragma once
#include <GL/glew.h>
#include <vector>
#include <string>
#include <glm.hpp>
class Skybox {
public:

    Skybox(const std::vector<std::string>& faces);

    // Render the skybox
    void Draw(const glm::mat4& view, const glm::mat4& projection);

private:
    GLuint skyboxVAO, skyboxVBO;
    GLuint cubeMapTexture;
    GLuint shaderProgram;

    // Internal methods
    GLuint loadCubeMap(const std::vector<std::string>& faces);
    GLuint loadShader(const char* vertexPath, const char* fragmentPath);
};