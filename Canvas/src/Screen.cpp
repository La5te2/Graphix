#include "Screen.h"
#include <glad/glad.h>
#include <iostream>
using namespace std;

double Screen::s_mouseX = 0.0;
double Screen::s_mouseY = 0.0;
bool Screen::s_mouseButtons[3] = { false, false, false };

Screen::Screen() : m_window(nullptr) {}

Screen::~Screen() {
    shutdown();
}

bool Screen::initialize(int width, int height, const char* title) {
    if (!glfwInit()) {
        std::cout << "Fail to initialize GLFW!" << std::endl;
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!m_window) {
        std::cout << "Fail to create Screen!" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(0); 
    glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);
    glfwSetMouseButtonCallback(m_window, mouseButtonCallback);
    glfwSetCursorPosCallback(m_window, cursorPosCallback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Fail to initialize GLAD!" << std::endl;
        return false;
    }

    std::cout << "Screen Created: " << width << "x" << height << std::endl;
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

    // 设置视口
    glViewport(0, 0, width, height);

    return true;
}

void Screen::shutdown() {
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    glfwTerminate();
    std::cout << "Screen Closed" << std::endl;
}

bool Screen::shouldClose() const {
    return glfwWindowShouldClose(m_window);
}

void Screen::swapBuffers() {
    glfwSwapBuffers(m_window);
}

void Screen::pollEvents() {
    glfwPollEvents();
}

// 鼠标输入方法实现
void Screen::getMousePosition(double& x, double& y) const {
    x = s_mouseX;
    y = s_mouseY;
}

bool Screen::isMouseButtonPressed(int button) const {
    if (button >= 0 && button < 3) {
        return s_mouseButtons[button];
    }
    return false;
}

void Screen::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void Screen::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button >= 0 && button < 3) {
        s_mouseButtons[button] = (action == GLFW_PRESS);
    }
}

void Screen::cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    s_mouseX = xpos;
    s_mouseY = ypos;
}