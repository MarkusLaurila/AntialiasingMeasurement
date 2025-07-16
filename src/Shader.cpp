
#include "../header/Shader.h"

using namespace std;

SHADER::SHADER(const char *vertexPath, const char *fragmentPath) {

	std::ifstream Verttest(vertexPath);
	if (!Verttest.is_open()) {
		std::cerr << "Vertex shader file not found: " << vertexPath << std::endl;
	}
	std::ifstream Fragtest(fragmentPath);
	if (!Fragtest.is_open()) {
		std::cerr << "Vertex shader file not found: " << vertexPath << std::endl;
	}

		std::string vertexCode; std::string fragmentCode;
		std::ifstream vShaderFile; std::ifstream fShaderFile;

		vShaderFile.exceptions(std::ifstream::badbit|std::ifstream::failbit);
		fShaderFile.exceptions(std::ifstream::badbit|std::ifstream::failbit);
		try
		{
			// Open files
			vShaderFile.open(vertexPath);
			fShaderFile.open(fragmentPath);
			std::stringstream vShaderStream, fShaderStream;
			// Read file's buffer contents into streams
			vShaderStream << vShaderFile.rdbuf();
			fShaderStream << fShaderFile.rdbuf();
			// close file handlers
			vShaderFile.close();
			fShaderFile.close();
			// Convert stream into string
			vertexCode = vShaderStream.str();
			fragmentCode = fShaderStream.str();
		}
		catch (std::ifstream::failure e)
		{
			std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
		}
		// std::cout << "Vertex Shader Source:\n" << vertexCode << std::endl;
		// std::cout << "Fragment Shader Source:\n" << fragmentCode << std::endl;
		const GLchar* vShaderCode = vertexCode.c_str();
		const GLchar* fShaderCode = fragmentCode.c_str();
		// 2. Compile shaders
		GLuint vertex, fragment;
		GLint success;
		GLchar infoLog[512];
		// Vertex Shader
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vShaderCode, NULL);
		glCompileShader(vertex);
		// Print compile errors if any
		glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vertex, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		}
		// Fragment Shader
		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, NULL);
		glCompileShader(fragment);

		glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(fragment, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
		}
		// Shader Program
		this->program = glCreateProgram();
		glAttachShader(this->program, vertex);
		glAttachShader(this->program, fragment);
		glLinkProgram(this->program);

		glGetProgramiv(this->program, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(this->program, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		}

		glDeleteShader(vertex);
		glDeleteShader(fragment);
}
void SHADER::UseShader() {
    glUseProgram(this->program);
}
