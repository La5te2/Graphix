#include "Shape.h"
#include <glm/gtc/matrix_transform.hpp>

glm::mat4 Shape::getTransformMatrix() const {
    glm::mat4 transform(1.0f);

    // 应用变换顺序：平移 -> 旋转 -> 缩放
    transform = glm::translate(transform, glm::vec3(m_position, 0.0f));
    transform = glm::rotate(transform, m_rotation, glm::vec3(0.0f, 0.0f, 1.0f));
    transform = glm::scale(transform, glm::vec3(m_scale, 1.0f));

    return transform;
}

void Shape::translate(const glm::vec2& offset) {
    m_position += offset;
}

void Shape::rotate(float angle) {
    m_rotation += angle;
}

void Shape::scale(const glm::vec2& factor) {
    m_scale *= factor;
}