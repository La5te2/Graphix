#pragma once
#include "Shape.h"

class Line : public Shape {
public:
    Line(const glm::vec2& start, const glm::vec2& end);
    ~Line();

    void draw() const override;
    ShapeType getType() const override { return ShapeType::LINE; }
    static void setProjectionMatrix(const glm::mat4& projection);

    glm::vec2 getStart() const { return m_start; }
    glm::vec2 getEnd() const { return m_end; }

private:
    glm::vec2 m_start;
    glm::vec2 m_end;

    unsigned int m_VAO{ 0 };
    unsigned int m_VBO{ 0 };
};