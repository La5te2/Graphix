#include "App.h"
#include "Objects.h"
#include <iostream>
#include <chrono>
#include <thread>

using namespace std;

App::App()
    : m_screen()
    , m_rotationAngle(0.0f)
    , m_scaleFactor(1.0f)
    , m_translation(0.0f, 0.0f)
    , m_scalingUp(true)
    , m_translatingUp(true)
    , m_frameCount(0)
    , m_fpsTimer(0.0f)
    , m_currentFPS(0.0f)
    , m_isRunning(false)
    , m_animationsEnabled(true) {
}

void App::run() {
    initialize();
    mainLoop();
    shutdown();
}

void App::initialize() {
    if (!m_screen.initialize(1920, 1080, "Engine")) {
        std::cout << "Fail to initialize Screen!" << std::endl;
        return;
    }
    m_screenWidth = 1920;
    m_screenHeight = 1080;
    glm::mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
    Line::setProjectionMatrix(projection);
    m_slider.initialize(1920, 1080);

    // 设置滑块位置在窗口底部
    float bottomMargin = 30.0f; // 距离底部的边距
    float sliderHeight = 20.0f; // 滑块高度
    float sliderY = m_screenHeight - bottomMargin - sliderHeight; // 计算Y坐标

    m_slider.setPosition(glm::vec2(10.0f, sliderY)); // 设置位置
    m_slider.setSize(glm::vec2(m_screenWidth - 20.0f, sliderHeight)); // 设置大小

    // 其他设置保持不变
    m_slider.setValueRange(12.0f, 300.0f);
    m_slider.setValue(m_animConfig.targetFPS);
    m_slider.setValueChangedCallback([this](float value) {
        this->onFPSChanged(value);
        });
    m_slider.setTrackColor(glm::vec3(0.3f, 0.3f, 0.3f));
    m_slider.setThumbColor(glm::vec3(0.8f, 0.8f, 0.8f));
    m_slider.setThumbHoverColor(glm::vec3(0.2f, 0.6f, 1.0f));
    m_lastFrameTime = std::chrono::steady_clock::now();

    testShapes();
    m_isRunning = true;
}
void App::onFPSChanged(float fps) {
    setFrameRate(fps);
}
void App::handleMouseInput() {
    // 获取鼠标状态
    double mouseX, mouseY;
    m_screen.getMousePosition(mouseX, mouseY);
    bool mousePressed = m_screen.isMouseButtonPressed(0);  // 0 = 左键
    m_slider.handleMouseInput(mouseX, mouseY, mousePressed);
}
void App::mainLoop() {
    while (m_isRunning && !m_screen.shouldClose()) {
        auto frameStart = std::chrono::steady_clock::now();

        // 计算 deltaTime
        auto currentTime = std::chrono::steady_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - m_lastFrameTime).count();
        m_lastFrameTime = currentTime;

        // 限制最大 deltaTime
        if (deltaTime > 0.1f) deltaTime = 0.1f;

        // 更新帧率统计
        updateFrameRateStats(deltaTime);
        m_screen.pollEvents();
        handleMouseInput();
        // 设置背景颜色和线条宽度
        glClearColor(m_animConfig.backgroundColor.r,
            m_animConfig.backgroundColor.g,
            m_animConfig.backgroundColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // 更新动画
        if (m_animationsEnabled) {
            updateRotationAnimation(deltaTime);
            updateScalingAnimation(deltaTime);
            updateTranslationAnimation(deltaTime);
        }

        // 应用变换到形状
        if (m_shapes.size() > 0 && m_animConfig.enableTranslation) {
            m_shapes[0]->setPosition(m_translation);
        }

        if (m_shapes.size() > 1 && m_animConfig.enableRotation) {
            m_shapes[1]->setRotation(m_rotationAngle);
        }

        if (m_shapes.size() > 2 && m_animConfig.enableScaling) {
            m_shapes[2]->setScale(glm::vec2(m_scaleFactor, m_scaleFactor));
        }
        glLineWidth(m_animConfig.lineWidth);
        for (const auto& shape : m_shapes) {
            shape->draw();
        }
        m_slider.render();
        m_screen.swapBuffers();
        if (m_animConfig.enableFrameRateLimit) {
            limitFrameRate(frameStart);
        }
    }
}

void App::updateRotationAnimation(float deltaTime) {
    m_rotationAngle += m_animConfig.rotationSpeed * deltaTime;
}

void App::updateScalingAnimation(float deltaTime) {
    if (m_scalingUp) {
        m_scaleFactor += m_animConfig.scaleSpeed * deltaTime;
        if (m_scaleFactor > m_animConfig.maxScale) m_scalingUp = false;
    }
    else {
        m_scaleFactor -= m_animConfig.scaleSpeed * deltaTime;
        if (m_scaleFactor < m_animConfig.minScale) m_scalingUp = true;
    }
}

void App::updateTranslationAnimation(float deltaTime) {
    if (m_translatingUp) {
        m_translation.y += m_animConfig.translationSpeed * deltaTime;
        if (m_translation.y > m_animConfig.maxTranslation) m_translatingUp = false;
    }
    else {
        m_translation.y -= m_animConfig.translationSpeed * deltaTime;
        if (m_translation.y < m_animConfig.minTranslation) m_translatingUp = true;
    }
}

void App::updateFrameRateStats(float deltaTime) {
    m_frameCount++;
    m_fpsTimer += deltaTime;

    if (m_fpsTimer >= 1.0f) {
        m_currentFPS = m_frameCount;
        std::cout << "FPS: " << m_currentFPS << std::endl;
        m_frameCount = 0;
        m_fpsTimer = 0.0f;
    }
}

void App::limitFrameRate(const std::chrono::steady_clock::time_point& frameStart) {
    using namespace std::chrono;

    auto frameEnd = steady_clock::now();
    auto frameDuration = frameEnd - frameStart;
    auto targetDuration = duration<float>(1.0f / m_animConfig.targetFPS);

    if (frameDuration < targetDuration) {
        auto sleepTime = targetDuration - frameDuration;
        auto sleepStart = steady_clock::now();
        while (steady_clock::now() - sleepStart < sleepTime) {
            std::this_thread::yield();
        }
    }
}

void App::setAnimationSpeed(float speedMultiplier) {
    m_animConfig.rotationSpeed *= speedMultiplier;
    m_animConfig.scaleSpeed *= speedMultiplier;
    m_animConfig.translationSpeed *= speedMultiplier;

    // 确保速度在合理范围内
    m_animConfig.rotationSpeed = glm::clamp(m_animConfig.rotationSpeed,
        m_animConfig.minRotationSpeed,
        m_animConfig.maxRotationSpeed);
}

void App::toggleAnimation(bool enabled) {
    m_animationsEnabled = enabled;
}

void App::resetAnimations() {
    m_rotationAngle = 0.0f;
    m_scaleFactor = 1.0f;
    m_translation = glm::vec2(0.0f, 0.0f);
    m_scalingUp = true;
    m_translatingUp = true;
}

void App::testShapes() {
    // 创建水平直线（用于平移测试）
    auto line1 = std::make_unique<Line>(glm::vec2(-0.8f, 0.0f), glm::vec2(0.8f, 0.0f));
    line1->setColor(glm::vec3(1.0f, 0.0f, 0.0f)); // 红色
    m_shapes.push_back(std::move(line1));

    // 创建垂直直线（用于旋转测试）
    auto line2 = std::make_unique<Line>(glm::vec2(0.0f, -0.5f), glm::vec2(0.0f, 0.5f));
    line2->setColor(glm::vec3(0.0f, 1.0f, 0.0f)); // 绿色
    m_shapes.push_back(std::move(line2));

    // 创建对角线（用于缩放测试）
    auto line3 = std::make_unique<Line>(glm::vec2(-0.6f, -0.6f), glm::vec2(0.6f, 0.6f));
    line3->setColor(glm::vec3(0.0f, 0.0f, 1.0f)); // 蓝色
    m_shapes.push_back(std::move(line3));
}

void App::shutdown() {
    std::cout << "closing..." << std::endl;
    m_isRunning = false;
    m_screen.shutdown();
}