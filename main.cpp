#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <random>
#include <array>
#include <map>
#include <tclap/CmdLine.h>
#include <filesystem>
#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <algorithm>
#include <thread>

#include "initialization.hpp"
#include "shader.hpp"
#include "vec.hpp"

std::string vertex_filename("vertex_shader.glsl"), fragment_filename("fragment_shader.glsl");

enum class movement {
	none,
	circle,
	velocity,
};

// *INDENT-OFF*
constexpr float vertices[] = { // vertices for a rectangle that fills the entire screen
		-1.f, -1.f, 0.0f,
		 1.f, -1.f, 0.0f,
		-1.f,  1.f, 0.0f,
		
		-1.f,  1.f, 0.0f,
		 1.f,  1.f, 0.0f,
		 1.f, -1.f, 0.0f
};
// *INDENT-ON*

void shaders_not_found_error() {
	std::cout << "Could not find shaders at either the path provided to -f, ~/.config, or $XDG_CONFIG_HOME\nExiting" <<
	          std::endl;
	std::exit(2);
}

bool shaders_are_here(std::string path) {
	std::ifstream vs(path + vertex_filename), fs(path + fragment_filename);
	return (vs.good() && fs.good());
}

std::string get_config_dir() {
	std::string env_config;
	if (std::getenv("XDG_CONFIG_HOME"))   // can return null pointer, and std::string cannot be initializated with one
		env_config = std::getenv("XDG_CONFIG_HOME");
	else
	{
		if (std::getenv("HOME")) {
			std::string home = std::getenv("HOME");
			env_config = home + "/.config";
		}
		else {
			std::cerr << "You are homeless, lol.  Either set $XDG_CONFIG_HOME or $HOME." << std::endl;
			std::exit(-1);
		}
	}
	env_config += "/gg2/";
	return env_config;  // should never be able to reach this.
}

void remove_newlines(std::string &input){
	std::replace(input.begin(),input.end(),'\n',' ');
}

int main(int argc, char **argv)
{

	int num_points;
	int num_used;
	vec2 point_margins;
	bool print_frame_times;
	bool use_grid_points;
	double point_vel_max;
	int_vec2 grid_dimensions;
	bool use_mouse_point;
	vec3 color_min;
	vec3 color_max;
	movement point_movement_type;
	std::string sorting_algo;
	bool sorting_algo_is_block;
	bool draw_circles;
	std::string value_algo;
	bool value_algo_is_block;
	bool ordering_greater;
	bool render_to_image;
	std::string image_filename;
	float color_rotation_speed;
	float angle_rotation_speed;
	float cirle_rotation_circumference;
	std::string prerun_block;
	int_vec2 window_size = {800, 600};
	std::string config_path;
	int multisampling;
	std::vector<std::string> extra_includes;
	int desired_framerate; 
	
	try {
		// *INDENT-OFF*
		TCLAP::CmdLine cmd("gg2 [args]");
		TCLAP::ValueArg<int> num_points_arg("n", "numpoints", "Total number of points.", 0,10, "integer");
		TCLAP::ValueArg<int> num_used_arg("a", "numavg","Total number of points used in the weighted average.", 0,2,"integer");
		TCLAP::ValueArg<vec2> margins_arg("m", "marg","Size of the margins where points will generally not be.",0,{0, 0}, "floatxfloat");
		TCLAP::SwitchArg print_frame_times_arg ("t", "times", "Print time of each frame.", false);
		TCLAP::ValueArg<int_vec2> use_grid_points_arg("g", "grid", "Arrange points in a WIDTHxHEIGHT grid", 0,{5, 5}, "intxint");
		TCLAP::ValueArg<vec3> color_max_arg("x", "max", "Maximum RGB value for each color", 0,{1, 1, 1},"floatxfloatxfloat");
		TCLAP::ValueArg<vec3> color_min_arg("i", "min", "Minimum RGB value for each color", 0,{0, 0, 0},"floatxfloatxfloat");
		TCLAP::ValueArg<float> point_vel_max_arg ("s", "speed", "Highest initial velocity of points when using velocities as the movement type", 0,0.01, "float");
		TCLAP::SwitchArg use_mouse_arg ("o", "mouse", "Position a point at the mouse",false);
		TCLAP::ValueArg<std::string> point_movement_type_arg ("y", "type","Type of movement Values are 'none', 'circle', and 'velocity'", 0, "velocity", "string");
		TCLAP::SwitchArg sorting_algo_is_block_arg ("E", "is-block", "Is the sorting algorithim provided meant to be an entire block.",false);
		TCLAP::ValueArg<std::string> sorting_algo_arg ("e", "sorter","Ranking algorithim. The variables 'dist', 'angle', 'point', and 'pc' are available.", 0, "dist", "string");
		TCLAP::SwitchArg value_algo_is_block_arg ("V", "vis-block", "Is the value algorithim provided meant to be an entire block.",false);
		TCLAP::ValueArg<std::string> value_algo_arg ("v", "valuer","Algorithim that is used for the weighted average of the colors. The same variables as the ranking algo are available, as well as `value`.", 0, "value", "string");
		TCLAP::SwitchArg draw_circles_arg ("d", "draw-circles", "Draw circles at the location of every point.",false);
		TCLAP::SwitchArg ordering_greater_arg ("G", "greater", "Cull the less valueable points, instead of the most valueable.",false);
		TCLAP::ValueArg<std::string> config_path_arg ("f", "filepath","Path to a directory containing vertex_shader.glsl and fragment_shader.glsl.", 0, get_config_dir(), "path");
		TCLAP::ValueArg<std::string> image_filename_arg ("I", "image","Write an image to this file, do not open a window.", 0, "", "path");
		TCLAP::ValueArg<int_vec2> window_size_arg("w", "window-size", "Size of window or image prodcued", 0,{600, 800}, "intxint");
		TCLAP::ValueArg<float> color_rotation_speed_arg ("r", "color-speed","Color rotation speed. Defaults to not rotating.", 0, 0, "float");
		TCLAP::ValueArg<float> angle_rotation_speed_arg ("R", "rotate-speed","Angle rotation speed.", 0, 0, "float");
		TCLAP::ValueArg<float> cirle_rotation_circumference_arg ("c", "circle-circ","Size of the circles when using -y circle", 0, 0.1, "float");
		TCLAP::ValueArg<std::string> prerun_block_arg ("p", "prerun-block","Code that runs at the very beginning of the shader.", 0, "", "string");
		TCLAP::ValueArg<int> multisampling_arg ("M", "multisampling","If specified, do multisampling with the specified amount of samples per pixel", 0, 0, "int");
		TCLAP::MultiArg<std::string> extra_includes_arg("U","include", "extra files to insert before the fragment shader", false, "file");
		TCLAP::ValueArg<int> desired_framerate_arg ("F", "fps","Allows you to limit your fps", 0, 0, "int");
	
		cmd.add(num_points_arg);
		cmd.add(num_used_arg);
		cmd.add(margins_arg);
		cmd.add(print_frame_times_arg); 
		cmd.add(use_grid_points_arg);
		cmd.add(color_max_arg);
		cmd.add(color_min_arg);
		cmd.add(point_vel_max_arg);
		cmd.add(use_mouse_arg);
		cmd.add(point_movement_type_arg);
		cmd.add(sorting_algo_is_block_arg);
		cmd.add(sorting_algo_arg);
		cmd.add(value_algo_arg);
		cmd.add(value_algo_is_block_arg);
		cmd.add(ordering_greater_arg); 
		cmd.add(color_rotation_speed_arg);
		cmd.add(draw_circles_arg);
		cmd.add(config_path_arg);
		cmd.add(image_filename_arg);
		cmd.add(window_size_arg);
		cmd.add(angle_rotation_speed_arg);
		cmd.add(cirle_rotation_circumference_arg);
		cmd.add(prerun_block_arg); 
		cmd.add(multisampling_arg); 
		cmd.add(extra_includes_arg);
		cmd.add(desired_framerate_arg);
		
		// *INDENT-ON*
		
		cmd.parse(argc, argv);
		
		num_points = num_points_arg.getValue();
		num_used = num_used_arg.getValue();
		point_margins = margins_arg.getValue();
		print_frame_times = print_frame_times_arg.getValue();
		color_max = color_max_arg.getValue();
		color_min = color_min_arg.getValue();
		point_vel_max = point_vel_max_arg.getValue();
		use_mouse_point = use_mouse_arg.getValue();
		std::string point_movement_string = point_movement_type_arg.getValue();
		sorting_algo_is_block = sorting_algo_is_block_arg.getValue();
		sorting_algo = sorting_algo_arg.getValue();
		value_algo_is_block = value_algo_is_block_arg.getValue();
		value_algo = value_algo_arg.getValue();
		color_rotation_speed = color_rotation_speed_arg.getValue();
		draw_circles = draw_circles_arg.getValue();
		ordering_greater = ordering_greater_arg.getValue();
		render_to_image = image_filename_arg.isSet();
		image_filename = image_filename_arg.getValue();
		window_size = window_size_arg.getValue();
		config_path = config_path_arg.getValue();
		angle_rotation_speed = angle_rotation_speed_arg.getValue();
		cirle_rotation_circumference = cirle_rotation_circumference_arg.getValue();
		prerun_block = prerun_block_arg.getValue();
		multisampling = multisampling_arg.getValue();
		extra_includes = extra_includes_arg.getValue();
		desired_framerate = desired_framerate_arg.getValue();
		
		if (! shaders_are_here(config_path)) {
			std::cout << "Shaders were not found at " <<
			          (config_path_arg.isSet() ? config_path : "XDG_CONFIG_HOME or ~/.config") << std::endl;
			std::exit(2);
		}
		
		if (use_grid_points_arg.isSet()) {
			use_grid_points = true;
			grid_dimensions = use_grid_points_arg.getValue();
			num_points = grid_dimensions.x * grid_dimensions.y;
		} else
			use_grid_points = false; // not really needed, but gets rid of a warning
			
		if (num_used > num_points) {
			std::cout << "Numavg cannot be bigger then numpoints!" << std::endl;
			std::exit(3);
		}
		
		std::map<std::string, movement> themap {{"none", movement::none}, {"circle", movement::circle}, {"velocity", movement::velocity}};
		try { point_movement_type = themap.at(point_movement_string); }
		catch (std::out_of_range &E) { throw TCLAP::ArgException(point_movement_string + "is not one of 'none', 'circle', or 'velocity'"); }
		
	} catch (TCLAP::ArgException &e) {
		std::cerr << "error:" << e.error() << std::endl;
		std::exit(1);
	}
	
	GLFWwindow *window = initialize_glfw(window_size.x, window_size.y, !render_to_image, multisampling);
	initialize_glad();
	if (multisampling)
		glEnable(GL_MULTISAMPLE);
		
	// Initializing and constructing the shaders
	Shader vertex_shader(GL_VERTEX_SHADER, "VERTEX"); // Vertex is simple, it barely does anything
	vertex_shader.read_file(config_path + vertex_filename);
	vertex_shader.compile();
	
	Shader fragment_shader(GL_FRAGMENT_SHADER, "FRAGMENT"); // Fragment shader needs to do most of the heavy lifting
	fragment_shader.source << "#version 460\n#define NUM_POINTS " << num_points <<
	                       "\n#define NUM_USED " << num_used << "\n";
	                       
	if (draw_circles)
		fragment_shader.source << "#define DRAW_CIRCLES\n";
	if (sorting_algo_is_block)
		fragment_shader.source << "#define SORT_USE_BODY\n";
	if (value_algo_is_block)
		fragment_shader.source << "#define VALUE_USE_BODY\n";
		
	fragment_shader.source << "#define ROTATE_COLORS " << color_rotation_speed << std::endl;
	fragment_shader.source << "#define ROTATE_ANGLE " << angle_rotation_speed << std::endl;
	fragment_shader.source << "#define ORDERING " << (ordering_greater ? ">" : "<") << std::endl;

	remove_newlines(prerun_block);
	remove_newlines(sorting_algo);
	remove_newlines(value_algo); 

	fragment_shader.source << "#define PRERUN_BLOCK " << prerun_block << std::endl;
	fragment_shader.source << "#define VALUE_ALGO " << value_algo << std::endl;
	fragment_shader.source << "#define SORT_ALGO " << sorting_algo << std::endl;

	for (auto i : extra_includes) {
		fragment_shader.read_file(i);
		std::cout << i << std::endl;
	}
	
	fragment_shader.source << "#line 1\n"; // Without this glsl compilation issues would have the wrong line numbers.
	
	// appends the contents of that file to the existing contents
	fragment_shader.read_file(config_path + fragment_filename);
	fragment_shader.compile();
	
	unsigned int shaderProgram = glCreateProgram();
	// Compile and link the shader program
	glAttachShader(shaderProgram, vertex_shader.id);
	glAttachShader(shaderProgram, fragment_shader.id);
	glLinkProgram(shaderProgram);
	check_link_success(shaderProgram);
	glDeleteShader(vertex_shader.id);
	glDeleteShader(fragment_shader.id);
	
	
	unsigned int VBO, VAO;
	// Set up vertex and array buffers
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO); // set type of buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
	
	GLuint fbo, render_buf;
	cv::Mat img(window_size.y, window_size.x, CV_8UC3);
	if (render_to_image) {
		glGenFramebuffers(1, &fbo); // Set up framebuffer
		glGenRenderbuffers(1, &render_buf);
		glBindRenderbuffer(GL_RENDERBUFFER, render_buf);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_BGRA, window_size.x, window_size.y);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, render_buf);
		glDeleteFramebuffers(1, &fbo);
		glDeleteRenderbuffers(1, &render_buf);
		
		glPixelStorei(GL_PACK_ALIGNMENT, (img.step & 3) ? 1 : 4); // Set up image.
		glPixelStorei(GL_PACK_ROW_LENGTH, img.step / img.elemSize());
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	}
	
	// Here is where the actual program starts
	std::default_random_engine rand_engine((std::random_device()()));
	std::uniform_real_distribution<> vel_max_rand(- point_vel_max, point_vel_max);
	
	std::vector<vec2> points(num_points);
	std::vector<float> point_speeds(num_points);
	std::vector<vec2> effective_points(num_points);
	std::vector<vec2> point_velocities(num_points);
	std::vector<vec3> point_colors(num_points);
	
	if (use_grid_points) {
		int i = 0;
		for (int x = 0; x < grid_dimensions.x; x++)
			for (int y = 0; y < grid_dimensions.y; y++) {
				points.at(i) = { static_cast<float>(x) / (grid_dimensions.x - 1), static_cast<float>(y) / (grid_dimensions.y - 1) };
				i++;
			}
	} else
		for (auto &i : points) {
			i = {
				std::uniform_real_distribution<GLfloat>(point_margins.x, 1 - point_margins.x)(rand_engine),
				std::uniform_real_distribution<GLfloat>(point_margins.y, 1 - point_margins.y)(rand_engine)
			};
		}
		
	for (auto &i : point_colors) {
		i = { // Populate with random colors
			std::uniform_real_distribution<float>(color_min.r, color_max.r)(rand_engine),
			std::uniform_real_distribution<float>(color_min.g, color_max.g)(rand_engine),
			std::uniform_real_distribution<float>(color_min.b, color_max.b)(rand_engine),
		};
	}
	
	for (auto &i : point_speeds)
		i = std::uniform_real_distribution<>(-1, 1)(rand_engine);
		
	// These will fail if the variables are not actually used in the progam, its not serious.
	int timeLocation = glGetUniformLocation(shaderProgram, "time");
	if (timeLocation == GL_INVALID_VALUE)
		std::cout << "failed at time\n" << std::endl;
	int pointsLocation = glGetUniformLocation(shaderProgram, "points");
	if (pointsLocation == GL_INVALID_VALUE)
		std::cout << "failed at points\n" << std::endl;
	int pointColorsLocation = glGetUniformLocation(shaderProgram, "point_colors");
	if (pointColorsLocation == GL_INVALID_VALUE)
		std::cout << "failed at colors\n" << std::endl;
	int windowSizeLocation = glGetUniformLocation(shaderProgram, "window_size");
	if (windowSizeLocation == GL_INVALID_VALUE)
		std::cout << "failed at window\n" << std::endl;
		
		
	double initial_time = glfwGetTime();
	unsigned int elapsed_frames = 0;
	
	if (point_movement_type == movement::velocity)
		for (auto &i : point_velocities)
			i = { static_cast<GLfloat>(vel_max_rand(rand_engine)), static_cast<GLfloat>(vel_max_rand(rand_engine))};
			
	effective_points = points;
	
	auto clamper = [](float in, float low, float high) {
		// returns -1 if in is less than low, 0 if its between, and 1 if its above high
		return (in > high) - (in < low);
	};
	
	glUseProgram(shaderProgram);
	
	while (!glfwWindowShouldClose(window)) {
		if (!render_to_image)
			glfwGetWindowSize(window, &window_size.x, &window_size.y);
		float time = glfwGetTime();
		auto time_cpp = std::chrono::high_resolution_clock::now();
		
		switch (point_movement_type) { // Here is where the points are moved according to point_movement_type
		case movement::circle:
			for (int i = 0; i < num_points; i++) { // move the points in a circle
				effective_points[i] = {
					static_cast<GLfloat>(points[i].x + (std::sin(time *point_vel_max *point_speeds[i]) * cirle_rotation_circumference)),
					static_cast<GLfloat>(points[i].y + (std::cos(time *point_vel_max *point_speeds[i]) * cirle_rotation_circumference))
				};
			}
			break;
		case movement::velocity:
			for (int i = 0; i < num_points; i ++) {
				effective_points[i].x += point_velocities[i].x;
				effective_points[i].y += point_velocities[i].y;
				
				point_velocities[i].x += -0.1 * point_vel_max * clamper(effective_points[i].x, point_margins.x, 1 - point_margins.x);
				point_velocities[i].y += -0.1 * point_vel_max * clamper(effective_points[i].y, point_margins.y, 1 - point_margins.y);
				
				point_velocities[i].x += std::sin(time + point_colors[i].r * 360) * point_vel_max / 100;
				point_velocities[i].y += std::cos(time + point_colors[i].g * 360) * point_vel_max / 100;
				
				if (point_velocities[i].x > point_vel_max) point_velocities[i].x *= 0.95;
				if (point_velocities[i].y > point_vel_max) point_velocities[i].y *= 0.95;
			}
			break;
		case movement::none:
			;
		}
		
		std::vector<vec2> true_points(num_points);
		
		for (int i = 0; i < num_points; i++) {
			true_points[i].x = window_size.x * effective_points[i].x;
			true_points[i].y = window_size.y * effective_points[i].y;
		}
		
		if (use_mouse_point) {
			double cursorx;
			double cursory;
			glfwGetCursorPos(window, &cursorx, &cursory);
			true_points[0] = {static_cast<GLfloat>(cursorx), window_size.y - static_cast<GLfloat>(cursory)};
		}
		
		// Push points to shader
		glUniform2fv(pointsLocation, num_points, (GLfloat *) true_points.data());
		glUniform3fv(pointColorsLocation, num_points, (GLfloat *) point_colors.data());
		glUniform1f(timeLocation, time);
		glUniform2i(windowSizeLocation, window_size.x, window_size.y);
		
		// Do all the stuff for opengl to actually do something
		processInput(window);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		
		if (!render_to_image) {
			glfwSwapBuffers(window);
			glfwPollEvents();
		}	else {
			glReadBuffer(GL_COLOR_ATTACHMENT0);
			glReadPixels(0, 0, img.cols, img.rows, GL_BGR, GL_UNSIGNED_BYTE, img.data);
			break;
		}
	
		if (print_frame_times) {
			elapsed_frames++;
			double time_change = glfwGetTime() - initial_time;
			
			if (time_change > 1) {
				std::cout << time_change / elapsed_frames << std::endl;
				elapsed_frames = 0;
				initial_time = glfwGetTime();
			}
		}
		if(desired_framerate){
			std::this_thread::sleep_until(time_cpp + std::chrono::milliseconds(1000/(desired_framerate))); 
		}
	}
	
	if (render_to_image) {
		cv::flip(img, img, 0);
		cv::imwrite(image_filename, img);
	}
	glfwTerminate();
	return 0;
}
