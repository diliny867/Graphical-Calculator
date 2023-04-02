#include "VAO.h"

void VAO::generate(GLuint& id) {
	glGenVertexArrays(1, &id);//generate attribute array
}

void VAO::addAttrib(GLuint id,/*VBO vbo, */GLuint layout, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* offset) {
	bind(id);
//	vbo.bind();
	glEnableVertexAttribArray(layout);
	glVertexAttribPointer(layout, size, type, normalized, stride, offset);
}

void VAO::setAttribDivisor(GLuint id, GLuint index, GLuint divisor){
	bind(id);
	glVertexAttribDivisor(index, divisor);
}

void VAO::bind(GLuint id) {
	glBindVertexArray(id);
}

void VAO::unbind() {
	glBindVertexArray(0);
}

void VAO::deleteIt(GLuint& id) {
	glDeleteVertexArrays(1, &id);
}