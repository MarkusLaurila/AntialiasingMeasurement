#include "../header/Display.h"
#include <iostream>
using namespace std;

DISPLAY::DISPLAY(): window(nullptr) {
    glfwInit();
}
DISPLAY::~DISPLAY() = default;



void DISPLAY::createWindow(int height,int width) {
    if (!glfwInit()) {
        std::cout << "Failed to initialize GLFW3" << std::endl;
        exit(EXIT_FAILURE);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE,GL_FALSE);
    this->window=glfwCreateWindow(800,600,"OpenGL Demo", nullptr,nullptr);
    if (this->window == nullptr)
    {
        std::cout << "Failed to open window. Fatal Error" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    //Hide cursor
    glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_DISABLED);
    glfwMakeContextCurrent(window);
    //1 cycle refresh
    glfwSwapInterval(1);


}
GLFWwindow* DISPLAY::getWindow() {
    return this->window;
}



