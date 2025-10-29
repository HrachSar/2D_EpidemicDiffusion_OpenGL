#include "resource_manager.h"

#include <sstream>
#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

std::map<std::string, shader> resource_manager::m_shaders;
std::map<std::string, texture> resource_manager::m_textures;

shader resource_manager::loadShader(const char* vShaderPath, const char* fShaderPath, const char* gShaderPath, const char* cShaderPath, std::string name) {
    m_shaders[name] = loadShaderFromFile(vShaderPath, fShaderPath, gShaderPath, cShaderPath);

    return m_shaders[name];
}
shader resource_manager::getShader(std::string name) {
    return m_shaders[name];
}

void resource_manager::clear() {
    for (auto iter : m_shaders) {
        glDeleteShader(iter.second.ID);
    }
    for (auto iter : m_textures) {
        glDeleteTextures(1, &iter.second.m_id);
    }
}

shader resource_manager::loadShaderFromFile(const char* vShaderPath, const char* fShaderPath, const char* gShaderPath, const char* cShaderPath) {
    std::string vShaderCode, fShaderCode, gShaderCode, cShaderCode;

    try {
        std::ifstream vShaderFile(vShaderPath);
        std::ifstream fShaderFile(fShaderPath);

        std::stringstream vShaderStream, fShaderStream;

        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();

        vShaderFile.close();
        fShaderFile.close();

        vShaderCode = vShaderStream.str();
        fShaderCode = fShaderStream.str();

        if (gShaderPath != nullptr) {
            std::ifstream gShaderFile(gShaderPath);
            std::stringstream gShaderStream;
            gShaderStream << gShaderFile.rdbuf();
            gShaderFile.close();
            gShaderCode = gShaderStream.str();
        }
        if (cShaderPath != nullptr) {
            std::ifstream cShaderFile(cShaderPath);
            std::stringstream cShaderStream;
            cShaderStream << cShaderFile.rdbuf();
            cShaderFile.close();
            cShaderCode = cShaderStream.str();
        }


    }
    catch (std::exception e) {
        std::cout << "Couldn't load shaders" << std::endl;
    }

    const char* vertexCode = vShaderCode.c_str();
    const char* fragmentCode = fShaderCode.c_str();
    const char* geometryCode = gShaderCode.c_str();
    const char* computeCode = cShaderCode.c_str();

    shader _shader;
    _shader.compile(vertexCode, fragmentCode, gShaderPath != nullptr ? geometryCode : nullptr, cShaderPath != nullptr ? computeCode : nullptr);

    return _shader;
}
texture resource_manager::loadTexture(const char* filePath, bool alpha, std::string name) {
    m_textures[name] = loadTextureFromFile(filePath, alpha);

    return m_textures[name];
}
texture resource_manager::getTexture(std::string name) {
    return m_textures[name];
}
texture resource_manager::loadTextureFromFile(const char* filePath, bool alpha) {
    texture _texture;

    if (alpha) {
        _texture.m_internalFormat = GL_RGBA;
        _texture.m_imageFormat = GL_RGBA;
    }
    stbi_set_flip_vertically_on_load(true);
    int width, height, nrChannels;

    unsigned char* data = stbi_load(filePath, &width, &height, &nrChannels, 0);
    if (data) {
        _texture.generate(width, height, data);
        stbi_image_free(data);
    }
    else {
        std::cout << "Couldn't load texture" << std::endl;
    }


    return _texture;

}






