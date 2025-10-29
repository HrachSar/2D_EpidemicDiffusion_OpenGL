#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <map>
#include <string>
#include "shader.h"
#include "texture.h"

class resource_manager {
public:
    static std::map<std::string, shader> m_shaders;
    static std::map<std::string, texture> m_textures;

    static shader loadShader(const char* vShaderPath, const char* fShaderPath, const char* gShaderPath, const char* cShaderPath, std::string name);
    static texture loadTexture(const char* filePath, bool alpha, std::string name);
    static shader getShader(std::string name);
    static texture getTexture(std::string name);
    static void clear();
private:
    static shader loadShaderFromFile(const char* vShaderPath, const char* fShaderPath, const char* gShaderPath = nullptr, const char* cShaderPath = nullptr);
    static texture loadTextureFromFile(const char* filePath, bool alpha);
};



#endif //RESOURCE_MANAGER_H
