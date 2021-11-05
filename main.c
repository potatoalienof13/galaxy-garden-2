#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <random>
#include <array>

struct XY {
	int x;
	int y;
};

struct vec2 {
	GLfloat x;
	GLfloat y;
};

struct vec3 {
	GLfloat r;
	GLfloat g;
	GLfloat b;
};

XY window_size{800, 600};
const int num_points = 10;
const int num_used = 5;

class Shader
{
public:
	std::stringstream source;
	GLint id;
	GLint type;
	std::string name;
	
	Shader(int shader_type, std::string shader_name_arg)
	{
		type = shader_type;
		name = shader_name_arg;
	}
	void compile()
	{
		id = glCreateShader(type);
		std::string string_source = source.str();
		const char *cc_source = string_source.c_str();
		glShaderSource(id, 1, &cc_source, NULL);
		glCompileShader(id);
		
		int  success;
		char infoLog[512];
		glGetShaderiv(id, GL_COMPILE_STATUS, &success);
		
		if (!success) {
			glGetShaderInfoLog(id, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::" << name << "::COMPILATION_FAILED\n" << infoLog << std::endl;
			std::exit(1);
		}
	}
	
	void read_file(std::string filename)
	{
		std::ifstream shader_file(filename);
		
		if (shader_file.good()) {
			source << shader_file.rdbuf();
			
		} else {
			std::cout << "failed to open file for shader " << name << std::endl;
			std::exit(-2);
		}
	}
};



void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
	window_size = {width, height};
	glViewport(0, 0, width, height);
}

float vertices[] = { // vertices for a rectangle that fills the entire screen
	-1.f, -1.f, 0.0f,
	    1.f, -1.f, 0.0f,
	    -1.f,  1.f, 0.0f,
	    
	    -1.f, 1.f,  0.0f,
	    1.f, 1.f,  0.0f,
	    1.f, -1.f,  0.0f
    };

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

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
	GLFWwindow *window =
	    glfwCreateWindow(window_size.x, window_size.y, "opengle", NULL, NULL);
	    
	if (!window) {
		std::cout << " window failed" << std::endl;
		glfwTerminate();
		return 9;
	}
	
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	
	Shader vertex_shader(GL_VERTEX_SHADER, "VERTEX");
	vertex_shader.read_file("vertex_shader.vs");
	vertex_shader.compile();
	
	Shader fragment_shader(GL_FRAGMENT_SHADER, "FRAGMENT");
	fragment_shader.source << "#version 400\n#define NUM_POINTS " << num_points << "\n#define NUM_USED " << num_used <<
	                       "\n";
	fragment_shader.read_file("fragment_shader.fs");
	fragment_shader.compile();
	
	unsigned int shaderProgram;
	
	shaderProgram = glCreateProgram();
	
	glAttachShader(shaderProgram, vertex_shader.id);
	glAttachShader(shaderProgram, fragment_shader.id);
	
	glLinkProgram(shaderProgram);
	
	check_link_success(shaderProgram);
	
	glDeleteShader(vertex_shader.id);
	glDeleteShader(fragment_shader.id);
	
	unsigned int VBO, VAO;
	
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	
	glBindVertexArray(VAO);
	
	glBindBuffer(GL_ARRAY_BUFFER, VBO); // set type of buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glBindVertexArray(0);
	
	// float time = glfwGetTime();
	
	int timeLocation = glGetUniformLocation(shaderProgram, "time");
	
	if (timeLocation == -1) {
		puts("failed at time ");
	}
	
	int pointsLocation = glGetUniformLocation(shaderProgram, "points");
	
	if (pointsLocation == -1) {
		puts("failed at points");
	}
	
	int pointColorsLocation = glGetUniformLocation(shaderProgram, "point_colors");
	
	if (pointColorsLocation == -1) {
		puts("failed at colors");
	}
	
	std::array<vec2, num_points> points;
	
	std::random_device r;
	std::default_random_engine rand_engine(r());
	std::uniform_real_distribution<> rand_one(0, 1);
	
	for (int i = 0; i < num_points; i++) {
		points[i].x = rand_one(rand_engine);
		points[i].y = rand_one(rand_engine);
	}
	
	std::array<vec3, num_points> point_colors;
	
	for (int i = 0; i < num_points; i++) {
		point_colors[i].r = rand_one(rand_engine);
		point_colors[i].g = rand_one(rand_engine);
		point_colors[i].b = rand_one(rand_engine);
	}
	
	std::array<float, num_points> point_speeds;
	std::uniform_real_distribution<> rand_one_to_neg_one(-1, 1);
	
	for (auto &i : point_speeds) {
		i = rand_one_to_neg_one(rand_engine);
	}
	
	glUseProgram(shaderProgram);
	
	
	while (!glfwWindowShouldClose(window)) {
	
		std::uniform_real_distribution<> rand_dis(-10, 10);
		
		std::array<vec2, num_points> effective_points;
		
		
		float time = glfwGetTime();
		
		for (int i = 0; i < num_points; i++) {
			effective_points[i].x = window_size.x * points[i].x + std::sin(time * 2 * point_speeds[i]) * 40;
			effective_points[i].y = window_size.y * points[i].y + std::cos(time * 2 * point_speeds[i]) * 40;
		}
		
		glUniform2fv(pointsLocation, num_points, (GLfloat *) effective_points.data());
		glUniform3fv(pointColorsLocation, num_points, (GLfloat *) point_colors.data());
		glUniform1f(timeLocation, time);
		
		processInput(window);
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		
		glUseProgram(shaderProgram);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		
		
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	
	glfwTerminate();
	return 0;
}

