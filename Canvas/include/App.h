#pragma once
#include "Screen.h"
#include "Shape.h"
#include "Slider.h"
#include <memory>
#include <vector>
#include <chrono>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
struct AnimationConfig {
    float rotationSpeed = 1.0f;        // 旋转速度（弧度/秒）
    float maxRotationSpeed = 5.0f;     // 最大旋转速度
    float minRotationSpeed = 0.1f;     // 最小旋转速度

    float scaleSpeed = 1.0f;           // 缩放速度（单位/秒）
    float maxScale = 1.5f;             // 最大缩放
    float minScale = 0.5f;             // 最小缩放

    float translationSpeed = 0.5f;     // 平移速度（单位/秒）
    float maxTranslation = 0.3f;       // 最大平移距离
    float minTranslation = -0.3f;      // 最小平移距离

    float targetFPS = 60.0f;           // 目标帧率
    bool enableFrameRateLimit = true;  // 是否启用帧率限制

    float lineWidth = 10.0f;           // 线条宽度
    glm::vec3 backgroundColor = glm::vec3(1.0f, 1.0f, 1.0f); // 背景颜色（白色）

    bool enableRotation = true;        // 启用旋转动画
    bool enableScaling = true;         // 启用缩放动画
    bool enableTranslation = true;     // 启用平移动画
};

class App {
public:
    App();
    ~App() = default;

    void run();

    AnimationConfig& getAnimationConfig() { return m_animConfig; }
    const AnimationConfig& getAnimationConfig() const { return m_animConfig; }
    void setAnimationConfig(const AnimationConfig& config) { m_animConfig = config; }
    void handleMouseInput();
    void setAnimationSpeed(float speedMultiplier);
    void toggleAnimation(bool enabled);
    void resetAnimations();

    void setFrameRate(float fps) {
		m_animConfig.targetFPS = fps;
    }
    void setBackgroundColor(const glm::vec3& color) {
		m_animConfig.backgroundColor = color;
    }
    void setLineWidth(float width) {
		m_animConfig.lineWidth = width;
    }
private:
    int m_screenWidth;
    int m_screenHeight;
private:
    void initialize();
    void mainLoop();
    void shutdown();
    void testShapes();

    void updateFrameRateStats(float deltaTime);
    void limitFrameRate(const std::chrono::steady_clock::time_point& frameStart);

    void updateRotationAnimation(float deltaTime);
    void updateScalingAnimation(float deltaTime);
    void updateTranslationAnimation(float deltaTime);

    void onFPSChanged(float fps);

private:
    Screen m_screen;
	Slider m_slider;
    std::vector<std::unique_ptr<Shape>> m_shapes;
    AnimationConfig m_animConfig;

    float m_rotationAngle;
    float m_scaleFactor;
    glm::vec2 m_translation;
    bool m_scalingUp;
    bool m_translatingUp;

    std::chrono::steady_clock::time_point m_lastFrameTime;
    int m_frameCount;
    float m_fpsTimer;
    float m_currentFPS;

    bool m_isRunning;
    bool m_animationsEnabled;
};