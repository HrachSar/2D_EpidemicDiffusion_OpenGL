#include "shader.h"

shader::shader() : ID(0) , computeID(0) {}


shader& shader::use() {
    glUseProgram(ID);
    return *this;
}
void shader::compile(const char* vertexPath, const char* fragmentPath, const char* geometryPath, const char* computePath) {
    unsigned int vertex, fragment, geometry, compute;

    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertexPath, NULL);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");

    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragmentPath, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");

    if (geometryPath != nullptr) {
        geometry = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometry, 1, &geometryPath, NULL);
        glCompileShader(geometry);
        checkCompileErrors(geometry, "GEOMETRY");
    }

    if (computePath != nullptr) {
        compute = glCreateShader(GL_COMPUTE_SHADER);
        glShaderSource(compute, 1, &computePath, NULL);
        glCompileShader(compute);
        checkCompileErrors(compute, "COMPUTE");
    }

    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    if (geometryPath != nullptr) {
        glAttachShader(ID, geometry);
    }
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");

    computeID = glCreateProgram();
    if (computePath != nullptr) {
        glAttachShader(computeID, compute);
    }
    glLinkProgram(computeID);
    checkCompileErrors(computeID, "COMPUTE_PROGRAM");

    glDeleteShader(vertex);
    glDeleteShader(fragment);
    if (geometryPath != nullptr) {
        glDeleteShader(geometry);
    }
    if (computePath != nullptr) {
        glDeleteShader(compute);
    }
}
void shader::setInt(const std::string& name, int value, bool useShader) {
    if (useShader)
        this->use();
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);

}
void shader::setFloat(const std::string& name, float value, bool useShader) {
    if (useShader)
        this->use();
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void shader::setFloat2(const std::string& name, float x, float y, bool useShader) {
    if (useShader)
        this->use();
    glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
}
void shader::setFloat2(const std::string& name, glm::vec2& val, bool useShader) {
    if (useShader)
        this->use();
    glUniform2f(glGetUniformLocation(ID, name.c_str()), val.x, val.y);
}
void shader::setFloat3(const std::string& name, float x, float y, float z, bool useShader) {
    if (useShader)
        this->use();
    glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
}
void shader::setFloat3(const std::string& name, glm::vec3& val, bool useShader) {
    if (useShader)
        this->use();
    glUniform3f(glGetUniformLocation(ID, name.c_str()), val.x, val.y, val.z);
}
void shader::setFloat4(const std::string& name, float x, float y, float z, float w, bool useShader) {
    if (useShader)
        this->use();
    glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
}
void shader::setFloat4(const std::string& name, glm::vec4& val, bool useShader) {
    if (useShader)
        this->use();
    glUniform4f(glGetUniformLocation(ID, name.c_str()), val.x, val.y, val.z, val.w);
}
void shader::setMat4x4(const std::string& name, glm::mat4x4& val, bool useShader) {
    if (useShader)
        this->use();
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(val));
}

void shader::checkCompileErrors(unsigned int object, std::string type) {
    int success;
    char log[1024];
    if (type != "PROGRAM") {
        glGetShaderiv(object, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(object, 1024, NULL, log);
            std::cout << "| ERROR::SHADER: Compile-time error: Type: " << type << "\n"
                << log << "\n -- --------------------------------------------------- -- "
                << std::endl;
        }
    }
    else {
        glGetProgramiv(object, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(object, 1024, NULL, log);
            std::cout << "| ERROR::PROGRAM: Linking-time error: Type: " << type << "\n"
                << log << "\n -- --------------------------------------------------- -- "
                << std::endl;
        }
    }
}
