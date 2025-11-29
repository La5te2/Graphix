#pragma once
#include <glm/glm.hpp>
enum class ShapeType {
    LINE,
    ELLIPSE,
    POLYGON,
    RECTANGLE
};
class Shape {
public:
    virtual ~Shape() = default;

    // 纯虚函数
    virtual void draw() const = 0;
    virtual ShapeType getType() const = 0;

    // 相对变换方法（累加）
    void translate(const glm::vec2& offset);
    void rotate(float angle);
    void scale(const glm::vec2& factor);

    // 绝对变换方法（设置）
    void setPosition(const glm::vec2& position) { m_position = position; }
    void setRotation(float angle) { m_rotation = angle; }
    void setScale(const glm::vec2& scale) { m_scale = scale; }

    // 获取当前变换
    glm::vec2 getPosition() const { return m_position; }
    float getRotation() const { return m_rotation; }
    glm::vec2 getScale() const { return m_scale; }

    // 设置颜色
    void setColor(const glm::vec3& color) { m_color = color; }
    glm::vec3 getColor() const { return m_color; }

protected:
    glm::vec2 m_position{ 0.0f, 0.0f };
    float m_rotation{ 0.0f };
    glm::vec2 m_scale{ 1.0f, 1.0f };
    glm::vec3 m_color{ 1.0f, 1.0f, 1.0f };

    glm::mat4 getTransformMatrix() const;
};