#pragma once
#include <glm/glm.hpp>
#include <string>
#include "Shader.h"
#include <functional>

class Slider {
public:
    using ValueChangedCallback = std::function<void(float)>; // 修改为 std::function

    Slider();
    ~Slider();

    void initialize(int screenWidth, int screenHeight);

    // 设置滑动条位置和大小
    void setPosition(const glm::vec2& position);
    void setSize(const glm::vec2& size);

    // 设置值范围
    void setValueRange(float minValue, float maxValue);

    // 设置当前值（会更新滑块位置）
    void setValue(float value);
    float getValue() const { return m_currentValue; }

    // 设置回调函数
    void setValueChangedCallback(const ValueChangedCallback& callback) {
        m_valueChangedCallback = callback;
    }

    // 设置颜色
    void setTrackColor(const glm::vec3& color);
    void setThumbColor(const glm::vec3& color);
    void setThumbHoverColor(const glm::vec3& color);

    // 渲染和输入处理
    void render();
    void handleMouseInput(double mouseX, double mouseY, bool mousePressed);

private:
    void setupShaders();
    void setupGeometry();
    bool isMouseOverThumb(double mouseX, double mouseY);

private:
    int m_screenWidth;
    int m_screenHeight;

    // OpenGL 对象
    unsigned int m_VAO;
    unsigned int m_VBO;
    unsigned int m_EBO;
    Shader m_shader;

    // 滑动条几何属性
    glm::vec2 m_position;
    glm::vec2 m_size;
    float m_sliderValue;      // 0.0 到 1.0
    bool m_sliderDragging;

    // 值范围
    float m_minValue;
    float m_maxValue;
    float m_currentValue;

    // 颜色
    glm::vec3 m_trackColor;
    glm::vec3 m_thumbColor;
    glm::vec3 m_thumbHoverColor;

    // 回调函数
    ValueChangedCallback m_valueChangedCallback;
};