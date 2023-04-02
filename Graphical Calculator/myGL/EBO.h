#pragma once

#include <glad/glad.h>

class EBO {
public:
	EBO() = delete;
	static void generate(GLuint& id);
	static void generate(GLuint& id, GLsizeiptr size, const void* indices, GLenum usage);
	static void setData(GLuint& id, GLsizeiptr size, const void* indices, GLenum usage);
	static void bind(GLuint& id);
	static void unbind();
	static void deleteIt(GLuint& id);
};