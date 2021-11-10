#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <random>
#include <array>
#include <tclap/CmdLine.h>

#include "initialization.hpp"
#include "shader.hpp"

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
int num_points = 4;
int num_used = 2;

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



float vertices[] = { // vertices for a rectangle that fills the entire screen
	-1.f, -1.f, 0.0f,
	    1.f, -1.f, 0.0f,
	    -1.f,  1.f, 0.0f,
	    
	    -1.f, 1.f,  0.0f,
	    1.f, 1.f,  0.0f,
	    1.f, -1.f,  0.0f
    };


int main()
{
	/*
	try {
		TCLAP::CmdLine cmd("haha yes this is a random library");
	
	} catch (TCLAP::ArgException &e){
		std::cerr << "error:" << e.error() << std::endl;
	}*/
	
	
	GLFWwindow *window = initialize_glfw(window_size.x, window_size.y);
	initialize_glad();
	
	
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
	
	
	std::vector<vec2> points(num_points);
	
	std::random_device r;
	std::default_random_engine rand_engine(r());
	std::uniform_real_distribution<> rand_one(0, 1);
	
	for (int i = 0; i < num_points; i++) {
		points[i].x = rand_one(rand_engine);
		points[i].y = rand_one(rand_engine);
	}
	
	std::vector<vec3> point_colors(num_points);
	
	for (int i = 0; i < num_points; i++) {
		point_colors[i].r = rand_one(rand_engine);
		point_colors[i].g = rand_one(rand_engine);
		point_colors[i].b = rand_one(rand_engine);
	}
	
	std::vector<float> point_speeds(num_points);
	std::uniform_real_distribution<> rand_one_to_neg_one(-1, 1);
	
	for (auto &i : point_speeds) {
		i = rand_one_to_neg_one(rand_engine);
	}
	
	glUseProgram(shaderProgram);
	
	std::vector<vec2> effective_points(num_points);
	
	
	
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
	
	double initial_time = glfwGetTime();
	unsigned int elapsed_frames = 0;
	
	while (!glfwWindowShouldClose(window)) {
	
		std::uniform_real_distribution<> rand_dis(-10, 10);
		
		float time = glfwGetTime();
		
		for (int i = 0; i < num_points; i++) { // move the points in a circle
			effective_points[i].x = window_size.x * points[i].x + std::sin(time * 1 * point_speeds[i]) * 1000;
			effective_points[i].y = window_size.y * points[i].y + std::cos(time * 1 * point_speeds[i]) * 1000;
		}
		
		glUniform2fv(pointsLocation, num_points, (GLfloat *) effective_points.data());
		glUniform3fv(pointColorsLocation, num_points, (GLfloat *) point_colors.data());
		glUniform1f(timeLocation, time / 4);
		
		processInput(window);
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		
		glfwSwapBuffers(window);
		glfwPollEvents();
		{
			elapsed_frames++;
			double time_change = glfwGetTime() - initial_time;
			
			if (time_change > 1) {
				std::cout << time_change / elapsed_frames << std::endl;
				elapsed_frames = 0;
				initial_time = glfwGetTime();
				
			}
		}
	}
	
	glfwTerminate();
	return 0;
}

