#include "Texture2D.h"

Texture2D::Texture2D(int width, int height){
    this->type = TextureType::diffuse;
    this->path = "";
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}
Texture2D::Texture2D(int width, int height, GLint internalformat, GLenum format, GLenum dataType){
    this->type = TextureType::diffuse;
    this->path = "";
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, internalformat, width, height, 0, format, dataType, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

Texture2D::Texture2D(int width, int height, GLint internalformat, GLenum format, GLenum dataType, const void* data){
    this->type = TextureType::diffuse;
    this->path = "";
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, internalformat, width, height, 0, format, dataType, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

Texture2D::Texture2D(const char* name, TextureType type) {
    this->type = type;
    this->path = name;
    glGenTextures(1, &id);//generate texture
    bind();
    // set the texture wrapping/filtering options (on currently bound texture)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(name, &width, &height, &nrChannels, 0);//load image
    if (data) {
        GLenum format;
        switch (nrChannels) {
        case 1:
            format = GL_RED;
            break;
        case 3:
            format = GL_RGB;
            break;
        case 4:
            format = GL_RGBA;
            break;
        default: 
            format = GL_RGB;
            break;
        }
        glTexImage2D(GL_TEXTURE_2D, 0, (GLint)format, width, height, 0, format, GL_UNSIGNED_BYTE, data);//set texture as loaded image
        glGenerateMipmap(GL_TEXTURE_2D);//generate all the necessary mipmaps
    }
    else {
        std::cout << "Failed to load image" << std::endl;
    }
    stbi_image_free(data);
}
Texture2D::Texture2D(const char* name, const char* directory, TextureType type) {
    this->type = type;
    std::string filename = directory + std::string("/") + name;
    this->path = filename;
    glGenTextures(1, &id);//generate texture
    bind();
    // set the texture wrapping/filtering options (on currently bound texture)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrChannels, 0);//load image
    if (data) {
        GLenum format;
        switch (nrChannels) {
        case 1:
            format = GL_RED;
            break;
        case 3:
            format = GL_RGB;
            break;
        case 4:
            format = GL_RGBA;
            break;
        default: 
            format = GL_RGB;
            break;
        }
        glTexImage2D(GL_TEXTURE_2D, 0, (GLint)format, width, height, 0, format, GL_UNSIGNED_BYTE, data);//set texture as loaded image
        glGenerateMipmap(GL_TEXTURE_2D);//generate all the necessary mipmaps
    }
    else {
        std::cout << "Failed to load image" << std::endl;
    }
    stbi_image_free(data);
}

void Texture2D::setTexImage(int width, int height, GLint internalformat, GLenum format, GLenum dataType) const{
    bind();
    glTexImage2D(GL_TEXTURE_2D, 0, internalformat, width, height, 0, format, dataType, NULL);
}

void Texture2D::setParameteri(GLenum parameterName, GLint parameter) const{
    bind();
    glTexParameteri(GL_TEXTURE_2D, parameterName, parameter);
}

void Texture2D::activate(GLenum textureUnit) const{
    glActiveTexture(GL_TEXTURE0+textureUnit);
    bind();
}
void Texture2D::deactivate() {    
    glActiveTexture(GL_TEXTURE0);
}

void Texture2D::bind() const{
    glBindTexture(GL_TEXTURE_2D, id);//bind texture
}
void Texture2D::unbind() {
    glBindTexture(GL_TEXTURE_2D, 0);//unbind texture
}