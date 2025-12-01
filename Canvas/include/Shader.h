#pragma once
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
class Shader {
public:
    Shader();
    ~Shader();

    bool loadFromSource(const std::string& vertexSource, const std::string& fragmentSource);
    bool loadFromFile(const std::string& vertexPath, const std::string& fragmentPath);
    void use() const;

    void setUniform(const std::string& name, const glm::mat4& matrix) const;
    void setUniform(const std::string& name, const glm::vec3& vector) const;
    void setUniform(const std::string& name, float value) const;

    unsigned int getID() const { return m_programID; }

private:
    unsigned int compileShader(const std::string& source, unsigned int type);
    bool checkCompileErrors(unsigned int shader, const std::string& type);

    unsigned int m_programID;
};