
#ifndef INITIALIZATION_H
#define INITIALIZATION_H

#include "glad/glad.h"
#include <GLFW/glfw3.h>

void processInput(GLFWwindow *window);

void check_link_success(unsigned int program);

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

GLFWwindow *initialize_glfw(int width, int height, bool show_window, int multisampling);

void initialize_glad();

#endif
