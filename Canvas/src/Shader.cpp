#include "Shader.h"
#include <glad/glad.h>
#include <fstream>
#include <sstream>
#include <iostream>

Shader::Shader() : m_programID(0) {}

Shader::~Shader() {
    if (m_programID) {
        glDeleteProgram(m_programID);
    }
}

bool Shader::loadFromSource(const std::string& vertexSource, const std::string& fragmentSource) {
    unsigned int vertexShader = compileShader(vertexSource, GL_VERTEX_SHADER);
    unsigned int fragmentShader = compileShader(fragmentSource, GL_FRAGMENT_SHADER);

    if (!vertexShader || !fragmentShader) {
        return false;
    }

    m_programID = glCreateProgram();
    glAttachShader(m_programID, vertexShader);
    glAttachShader(m_programID, fragmentShader);
    glLinkProgram(m_programID);

    if (!checkCompileErrors(m_programID, "PROGRAM")) {
        glDeleteProgram(m_programID);
        m_programID = 0;
        return false;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return true;
}
bool Shader::loadFromFile(const std::string& vertexPath, const std::string& fragmentPath) {
    // 读取顶点着色器文件
    std::string vertexCode;
    std::ifstream vertexFile(vertexPath);
    if (vertexFile.is_open()) {
        std::stringstream vertexStream;
        vertexStream << vertexFile.rdbuf();
        vertexCode = vertexStream.str();
        vertexFile.close();
    }
    else {
        std::cout << "Fail to open vertex shader file: " << vertexPath << std::endl;
        return false;
    }

    // 读取片段着色器文件
    std::string fragmentCode;
    std::ifstream fragmentFile(fragmentPath);
    if (fragmentFile.is_open()) {
        std::stringstream fragmentStream;
        fragmentStream << fragmentFile.rdbuf();
        fragmentCode = fragmentStream.str();
        fragmentFile.close();
    }
    else {
        std::cout << "Fail to open fragment shader file: " << fragmentPath << std::endl;
        return false;
    }

    // 使用从文件读取的源码加载着色器
    return loadFromSource(vertexCode, fragmentCode);
}
void Shader::use() const {
    glUseProgram(m_programID);
}

unsigned int Shader::compileShader(const std::string& source, unsigned int type) {
    unsigned int shader = glCreateShader(type);
    const char* sourceCStr = source.c_str();
    glShaderSource(shader, 1, &sourceCStr, nullptr);
    glCompileShader(shader);

    std::string shaderType = (type == GL_VERTEX_SHADER) ? "VERTEX" : "FRAGMENT";
    if (!checkCompileErrors(shader, shaderType)) {
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

bool Shader::checkCompileErrors(unsigned int shader, const std::string& type) {
    int success;
    char infoLog[1024];

    if (type == "PROGRAM") {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
            std::cout << "Link Errors: " << infoLog << std::endl;
            return false;
        }
    }
    else {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
            std::cout << type << " Compile Errors: " << infoLog << std::endl;
            return false;
        }
    }

    return true;
}

void Shader::setUniform(const std::string& name, const glm::mat4& matrix) const {
    glUniformMatrix4fv(glGetUniformLocation(m_programID, name.c_str()), 1, GL_FALSE, &matrix[0][0]);
}

void Shader::setUniform(const std::string& name, const glm::vec3& vector) const {
    glUniform3fv(glGetUniformLocation(m_programID, name.c_str()), 1, &vector[0]);
}

void Shader::setUniform(const std::string& name, float value) const {
    glUniform1f(glGetUniformLocation(m_programID, name.c_str()), value);
}