#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#define	SHADER_NOT_INTERPOLATE_FLAG 1

class Shader
{
public:
	// the program id
	GLuint id;

	// constructor reads and builds the shader
	Shader(const char* vertexPath, const char* fragmentPath);
	Shader(const char* vertexPath, const char* geometryPath, const char* fragmentPath);
	Shader(const char* vertexPath, const char* fragmentPath, int flags);
	Shader(const char* vertexPath, const char* geometryPath, const char* fragmentPath, int flags);

	// use/activate the shader
	void use() const;

	// utility uniform functions
	GLuint getUniformIndex(const std::string& name) const;

	void bindUniformBlock(GLuint index, GLuint bindingPoint) const;

	void setBool(const std::string& name, bool value) const;
	void setInt(const std::string& name, int value) const;
	void setFloat(const std::string& name, float value) const;

	void setVec2(const std::string& name, glm::vec2& value) const;
	void setVec2(const std::string& name, float x, float y) const;

	void setVec3(const std::string& name, const glm::vec3& value) const;
	void setVec3(const std::string& name, float x, float y, float z) const;

	void setVec4(const std::string& name, const glm::vec4& value) const;
	void setVec4(const std::string& name, float x, float y, float z, float w) const;

	void setMat2(const std::string& name, const glm::mat2& mat) const;
	void setMat3(const std::string& name, const glm::mat3& mat) const;
	void setMat4(const std::string& name, const glm::mat4& mat) const;
private:
	std::size_t static find_pos_for_defines(const std::string& str);
};
