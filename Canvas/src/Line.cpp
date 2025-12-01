#include "Line.h"
#include "Shader.h"
#include <glad/glad.h>
#include <iostream>

// source code of line shader
const std::string vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec2 aPos;

    uniform mat4 projection;
    uniform mat4 model;

    void main() {
        gl_Position = projection * model * vec4(aPos, 0.0, 1.0);
    }
)";

const std::string fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;
    uniform vec3 lineColor;

    void main() {
        FragColor = vec4(lineColor, 1.0);
    }
)";
static Shader lineShader;
static bool shaderInitialized = false;
static glm::mat4 projectionMatrix = glm::mat4(1.0f);
void Line::setProjectionMatrix(const glm::mat4& projection) {
    projectionMatrix = projection;
}

Line::Line(const glm::vec2& start, const glm::vec2& end)
    : m_start(start), m_end(end) {
    if (!shaderInitialized) {
        if (lineShader.loadFromSource(vertexShaderSource, fragmentShaderSource)) {
            shaderInitialized = true;
        }
        else {
            std::cout << "Fail to initialize shader" << std::endl;
        }
    }

    // 设置顶点数据
    float vertices[] = {
        m_start.x, m_start.y,
        m_end.x, m_end.y
    };

    // 生成缓冲对象
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);

    // 绑定VAO
    glBindVertexArray(m_VAO);

    // 绑定VBO并上传数据
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 设置顶点属性指针
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // 解绑
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

Line::~Line() {
    if (m_VAO) {
        glDeleteVertexArrays(1, &m_VAO);
        m_VAO = 0;
    }
    if (m_VBO) {
        glDeleteBuffers(1, &m_VBO);
        m_VBO = 0;
    }
}

void Line::draw() const {
    if (!shaderInitialized) {
        std::cout << "Shader NOT Initialized" << std::endl;
        return;
    }

    // 使用着色器
    lineShader.use();

    // 设置颜色
    lineShader.setUniform("lineColor", m_color);

    // 设置模型矩阵（变换）
    auto model = getTransformMatrix();
    lineShader.setUniform("model", model);

    // 设置投影矩阵
    lineShader.setUniform("projection", projectionMatrix);

    // 绘制
    glBindVertexArray(m_VAO);
    glDrawArrays(GL_LINES, 0, 2);
    glBindVertexArray(0);
}