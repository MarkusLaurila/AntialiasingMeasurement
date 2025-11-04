
#pragma once

#include <GL/glew.h>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include "vec2.hpp"

class SHADER {
    public:
    GLuint program;
    SHADER(const char* vertexPath, const char* fragmentPath);
    void UseShader();
    void setBool(const std::string& name, bool value) const;
    void setInt(const char* name, int value);
    void setFloat(const char* name, float value);
    void setVec2(const char* name, const glm::vec2& value);


};

