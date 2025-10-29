#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <string>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class shader {
public:
    unsigned int ID;
    unsigned int computeID;
    shader();

    shader& use();
    void compile(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr, const char* computePath = nullptr);
    void checkCompileErrors(unsigned int object, std::string type);
    void setInt(const std::string& name, int value, bool useShader = false);
    void setFloat(const std::string& name, float value, bool useShader = false);
    void setFloat2(const std::string& name, float x, float y, bool useShader = false);
    void setFloat2(const std::string& name, glm::vec2& val, bool useShader = false);
    void setFloat3(const std::string& name, float x, float y, float z, bool useShader = false);
    void setFloat3(const std::string& name, glm::vec3& val, bool useShader = false);
    void setFloat4(const std::string& name, float x, float y, float z, float w, bool useShader = false);
    void setFloat4(const std::string& name, glm::vec4& val, bool useShader = false);
    void setMat4x4(const std::string& name, glm::mat4x4& val, bool useShader = false);
};



#endif //SHADER_H
