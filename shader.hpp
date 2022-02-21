#ifndef shader_hpp_INCLUDED
#define shader_hpp_INCLUDED

#include <sstream>
#include <GLFW/glfw3.h>


class Shader
{
public:
	std::stringstream source;
	GLint id;
	GLint type;
	std::string name;
	
	Shader(int shader_type, std::string shader_name_arg);
	
	void compile();
	
	void read_file(std::string filename);

	void set_version(std::string); 
};

#endif // shader_hpp_INCLUDED

