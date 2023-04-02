#pragma once

#include <glad/glad.h>

#include "VBO.h"

class VAO {
public:
	VAO() = delete;
	static void generate(GLuint& id);
	static void addAttrib(GLuint id, GLuint layout, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* offset);
	static void setAttribDivisor(GLuint id, GLuint index, GLuint divisor);
	static void bind(GLuint id);
	static void unbind();
	static void deleteIt(GLuint& id);
};