#pragma once

#include <glad/glad.h>

class VBO {
public:
	VBO() = delete;
	static void generate(GLuint& id);
	static void generate(GLuint& id, GLsizeiptr size, const void* data, GLenum usage);
	static void setData(GLuint& id, GLsizeiptr size, const void* data, GLenum usage);
	static void setSubData(GLuint& id, GLintptr offset, GLsizeiptr size, const void* data);
	static void bind(GLuint& id);
	static void unbind();
	static void deleteIt(GLuint& id);
};