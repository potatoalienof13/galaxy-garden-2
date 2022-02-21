#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include "shader.hpp"

Shader::Shader(int shader_type, std::string shader_name_arg) : type(shader_type), name(shader_name_arg){}

void Shader::compile()
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

void Shader::read_file(std::string filename)
{
	std::ifstream shader_file(filename);
	std::cout << filename << std::endl; 
	if (shader_file.good()) {
		source << shader_file.rdbuf();
		
	} else {

		std::cout << shader_file.rdstate() << std::endl; 
		std::cout << "failed to open file for shader " << name << std::endl;
		std::exit(-2);
	}
}

void Shader::set_version(std::string version){
	source << "#version " << version << std::endl; 
}

