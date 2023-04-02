#include "VBO.h"

void VBO::generate(GLuint& id) {
	glGenBuffers(1, &id);
}

void VBO::generate(GLuint& id, GLsizeiptr size, const void* data, GLenum usage) {
	glGenBuffers(1, &id);//generate vertex buffer
	bind(id);//bind the newly created buffer to the GL_ARRAY_BUFFER target (we can bind multible buffers to one target)
	glBufferData(GL_ARRAY_BUFFER, size, data, usage);//this is a function specifically targeted to copy user-defined data into the currently bound buffer
}

void VBO::setData(GLuint& id, GLsizeiptr size, const void* data, GLenum usage) {
	bind(id);//bind the newly created buffer to the GL_ARRAY_BUFFER target (we can bind multible buffers to one target)
	glBufferData(GL_ARRAY_BUFFER, size, data, usage);//this is a function specifically targeted to copy user-defined data into the currently bound buffer
}

void VBO::setSubData(GLuint& id, GLintptr offset, GLsizeiptr size, const void* data){
	bind(id);
	glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
}

void VBO::bind(GLuint& id) {
	glBindBuffer(GL_ARRAY_BUFFER, id);//bind buffer
}

void VBO::unbind() {
	glBindBuffer(GL_ARRAY_BUFFER, 0);//unbind buffer
}

void VBO::deleteIt(GLuint& id) {
	glDeleteBuffers(1, &id);
}