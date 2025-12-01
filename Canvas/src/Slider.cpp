#include "Slider.h"
#include <glad/glad.h>
#include <iostream>
#include <vector>
#include <cmath>

Slider::Slider()
    : m_screenWidth(1920)
    , m_screenHeight(1080)
    , m_VAO(0)
    , m_VBO(0)
    , m_EBO(0)
    , m_position(100.0f, 580.0f)
    , m_size(600.0f, 20.0f)
    , m_sliderValue(0.5f)
    , m_sliderDragging(false)
    , m_minValue(0.0f)
    , m_maxValue(1.0f)
    , m_currentValue(0.5f)
    , m_trackColor(0.5f, 0.5f, 0.5f)
    , m_thumbColor(0.8f, 0.8f, 0.8f)
    , m_thumbHoverColor(0.2f, 0.6f, 1.0f)
    {}

Slider::~Slider() {
    if (m_VAO) {
        glDeleteVertexArrays(1, &m_VAO);
    }
    if (m_VBO) {
        glDeleteBuffers(1, &m_VBO);
    }
    if (m_EBO) {
        glDeleteBuffers(1, &m_EBO);
    }
}

void Slider::initialize(int screenWidth, int screenHeight) {
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;
    setupShaders();
    setupGeometry();
}

void Slider::setPosition(const glm::vec2& position) {
    m_position = position;
}

void Slider::setSize(const glm::vec2& size) {
    m_size = size;
}

void Slider::setValueRange(float minValue, float maxValue) {
    m_minValue = minValue;
    m_maxValue = maxValue;

    // 更新当前值到新的范围
    float normalizedValue = (m_currentValue - m_minValue) / (m_maxValue - m_minValue);
    m_currentValue = m_minValue + normalizedValue * (maxValue - minValue);
}

void Slider::setValue(float value) {
    m_currentValue = glm::clamp(value, m_minValue, m_maxValue);
    m_sliderValue = (m_currentValue - m_minValue) / (m_maxValue - m_minValue);
}

void Slider::setTrackColor(const glm::vec3& color) {
    m_trackColor = color;
}

void Slider::setThumbColor(const glm::vec3& color) {
    m_thumbColor = color;
}

void Slider::setThumbHoverColor(const glm::vec3& color) {
    m_thumbHoverColor = color;
}

void Slider::setupShaders() {
    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        uniform mat4 projection;
        void main() {
            gl_Position = projection * vec4(aPos, 0.0, 1.0);
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        uniform vec3 uColor;
        void main() {
            FragColor = vec4(uColor, 1.0);
        }
    )";

    if (!m_shader.loadFromSource(vertexShaderSource, fragmentShaderSource)) {
        std::cout << "Failed to load UI shaders" << std::endl;
    }
}

void Slider::setupGeometry() {
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);
}

void Slider::render() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_shader.use();

    glm::mat4 projection = glm::ortho(0.0f, (float)m_screenWidth, (float)m_screenHeight, 0.0f, -1.0f, 1.0f);
    m_shader.setUniform("projection", projection);

    // 渲染滑动条背景
    float bgVertices[] = {
        m_position.x, m_position.y,
        m_position.x + m_size.x, m_position.y,
        m_position.x + m_size.x, m_position.y + m_size.y,
        m_position.x, m_position.y + m_size.y
    };

    unsigned int bgIndices[] = { 0, 1, 2, 2, 3, 0 };

    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(bgVertices), bgVertices, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(bgIndices), bgIndices, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    m_shader.setUniform("uColor", glm::vec3(0.3f, 0.3f, 0.3f));
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // 渲染轨道
    float trackHeight = 6.0f;
    float trackY = m_position.y + (m_size.y - trackHeight) / 2.0f;

    float trackVertices[] = {
        m_position.x, trackY,
        m_position.x + m_size.x, trackY,
        m_position.x + m_size.x, trackY + trackHeight,
        m_position.x, trackY + trackHeight
    };

    glBufferData(GL_ARRAY_BUFFER, sizeof(trackVertices), trackVertices, GL_DYNAMIC_DRAW);
    m_shader.setUniform("uColor", m_trackColor);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // 渲染滑块（圆形）
    float thumbX = m_position.x + m_sliderValue * m_size.x;
    float thumbY = m_position.y + m_size.y / 2.0f;
    float thumbRadius = 12.0f;

    std::vector<float> circleVertices;
    std::vector<unsigned int> circleIndices;

    circleVertices.push_back(thumbX);
    circleVertices.push_back(thumbY);

    int segments = 32;
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * 3.14159f * i / segments;
        circleVertices.push_back(thumbX + cos(angle) * thumbRadius);
        circleVertices.push_back(thumbY + sin(angle) * thumbRadius);
    }

    for (int i = 1; i <= segments; i++) {
        circleIndices.push_back(0);
        circleIndices.push_back(i);
        circleIndices.push_back(i + 1);
    }

    // 绘制填充的圆形
    glBufferData(GL_ARRAY_BUFFER, circleVertices.size() * sizeof(float), circleVertices.data(), GL_DYNAMIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, circleIndices.size() * sizeof(unsigned int), circleIndices.data(), GL_DYNAMIC_DRAW);

    if (m_sliderDragging) {
        m_shader.setUniform("uColor", m_thumbHoverColor);
    }
    else {
        m_shader.setUniform("uColor", m_thumbColor);
    }

    glDrawElements(GL_TRIANGLES, circleIndices.size(), GL_UNSIGNED_INT, 0);

    // 绘制边框
    std::vector<float> borderVertices;
    for (int i = 1; i <= segments + 1; i++) {
        borderVertices.push_back(circleVertices[i * 2]);
        borderVertices.push_back(circleVertices[i * 2 + 1]);
    }

    std::vector<unsigned int> borderIndices;
    for (int i = 0; i < segments; i++) {
        borderIndices.push_back(i);
        borderIndices.push_back((i + 1) % segments);
    }

    glBufferData(GL_ARRAY_BUFFER, borderVertices.size() * sizeof(float), borderVertices.data(), GL_DYNAMIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, borderIndices.size() * sizeof(unsigned int), borderIndices.data(), GL_DYNAMIC_DRAW);

    m_shader.setUniform("uColor", glm::vec3(0.2f, 0.2f, 0.2f));
    glDrawElements(GL_LINES, borderIndices.size(), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glDisable(GL_BLEND);
}

void Slider::handleMouseInput(double mouseX, double mouseY, bool mousePressed) {
    if (mousePressed) {
        if (isMouseOverThumb(mouseX, mouseY) || m_sliderDragging) {
            m_sliderDragging = true;

            // 更新滑块位置
            float relativeX = static_cast<float>(mouseX) - m_position.x;
            float newSliderValue = glm::clamp(relativeX / m_size.x, 0.0f, 1.0f);

            // 如果值发生变化
            if (newSliderValue != m_sliderValue) {
                m_sliderValue = newSliderValue;
                m_currentValue = m_minValue + m_sliderValue * (m_maxValue - m_minValue);

                // 调用回调函数
                if (m_valueChangedCallback) {
                    m_valueChangedCallback(m_currentValue);
                }
            }
        }
    }
    else {
        m_sliderDragging = false;
    }
}

bool Slider::isMouseOverThumb(double mouseX, double mouseY) {
    float thumbX = m_position.x + m_sliderValue * m_size.x;
    float thumbY = m_position.y + m_size.y / 2.0f;
    float thumbRadius = 12.0f;

    float dx = static_cast<float>(mouseX) - thumbX;
    float dy = static_cast<float>(mouseY) - thumbY;
    float distance = sqrt(dx * dx + dy * dy);

    return distance <= thumbRadius;
}