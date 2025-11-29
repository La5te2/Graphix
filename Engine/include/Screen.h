#pragma once
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

class Screen {
public:
    Screen();
    ~Screen();

    bool initialize(int width, int height, const char* title);
    void shutdown();

    bool shouldClose() const;
    void swapBuffers();
    void pollEvents();

    void getMousePosition(double& x, double& y) const;
    bool isMouseButtonPressed(int button) const;

private:
    GLFWwindow* m_window;

    // 静态回调函数
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);

    // 鼠标回调函数
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);

    // 静态鼠标状态变量（因为回调函数必须是静态的）
    static double s_mouseX;
    static double s_mouseY;
    static bool s_mouseButtons[3];  // 0=左键, 1=右键, 2=中键
};