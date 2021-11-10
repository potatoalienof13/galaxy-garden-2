#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <iostream>


void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}

void check_link_success(unsigned int program)
{
	int  success;
	char infoLog[512];
	
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	
	if (!success) {
		glGetProgramInfoLog(program, 512, NULL, infoLog);
		
		std::cout << "ERROR::SHADER::LIONING::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
}



void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
	glViewport(0, 0, width, height);
}

GLFWwindow *initialize_glfw(int width, int height)
{

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
	GLFWwindow *window =
	    glfwCreateWindow(width, height, "opengle", NULL, NULL);
	    
	if (!window) {
		std::cout << " window failed" << std::endl;
		glfwTerminate();
		std::exit(9);
	}
	
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	return window;
}

void initialize_glad()
{

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		std::exit(8);
	}
	
}
