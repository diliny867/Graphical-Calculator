#include "EBO.h"

void EBO::generate(GLuint& id) {
	glGenBuffers(1, &id);
}

void EBO::generate(GLuint& id, GLsizeiptr size, const void* indices, GLenum usage) {
	glGenBuffers(1, &id);//generate vertex buffer
	EBO::bind(id);//bind the newly created buffer to the GL_ARRAY_BUFFER target (we can bind multible buffers to one target)
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indices, usage);//this is a function specifically targeted to copy user-defined data into the currently bound buffer
}

void EBO::setData(GLuint& id, GLsizeiptr size, const void* indices, GLenum usage) {
	EBO::bind(id);//bind the newly created buffer to the GL_ARRAY_BUFFER target (we can bind multible buffers to one target)
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indices, usage);//this is a function specifically targeted to copy user-defined data into the currently bound buffer
}

void EBO::bind(GLuint& id) {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);//bind buffer
}

void EBO::unbind() {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);//unbind buffer
}

void EBO::deleteIt(GLuint& id) {
	glDeleteBuffers(1, &id);
}