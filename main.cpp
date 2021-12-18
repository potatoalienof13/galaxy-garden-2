#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <random>
#include <array>
#include <tclap/CmdLine.h>
#include <filesystem>
#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>

#include "initialization.hpp"
#include "shader.hpp"
#include "vec.hpp"

std::string vertex_filename("vertex_shader.glsl"), fragment_filename("fragment_shader.glsl");

enum class movement {
	none,
	circle,
	velocites,
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
		env_config = "~/.config";
	env_config += "/gg2/";
	return env_config;  // should never be able to reach this.
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
	bool rotate_colors;
	bool draw_circles;
	std::string value_algo;
	bool value_algo_is_block;
	bool ordering_greater;
	bool render_to_image;
	std::string image_filename;
	
	int_vec2 window_size = {800, 600};
	std::string config_path;
	
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
		TCLAP::ValueArg<std::string> value_algo_arg ("v", "valuer","Algorithim that is used for the weighted average of the colors. The same variables as the ranking algo are available, as well as `value`.", 0, "dist", "string");
		TCLAP::SwitchArg rotate_colors_arg ("r", "rotate-colors", "Fade between different colors.",false);
		TCLAP::SwitchArg draw_circles_arg ("d", "draw-circles", "Draw circles at the location of every point.",false);
		TCLAP::SwitchArg ordering_greater_arg ("G", "greater", "Cull the less valueable points, instead of the most valueable.",false);
		TCLAP::ValueArg<std::string> config_path_arg ("f", "filepath","Path to a directory containing vertex_shader.glsl and fragment_shader.glsl.", 0, get_config_dir(), "path");
		TCLAP::ValueArg<std::string> image_filename_arg ("I", "image","Write an image to this file, do not open a window.", 0, "", "path");
		
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
		cmd.add(rotate_colors_arg);
		cmd.add(draw_circles_arg);
		cmd.add(config_path_arg);
		cmd.add(image_filename_arg);
		// *INDENT-ON*
		
		cmd.parse(argc, argv);
		
		num_points = num_points_arg.getValue();
		num_used = num_used_arg.getValue();
		point_margins = margins_arg.getValue();
		print_frame_times = print_frame_times_arg.getValue();
		
		if (use_grid_points_arg.isSet()) {
			use_grid_points = true;
			grid_dimensions = use_grid_points_arg.getValue();
		} else
			use_grid_points = false; // not really needed, but gets rid of a warning
			
		color_max = color_max_arg.getValue();
		color_min = color_min_arg.getValue();
		point_vel_max = point_vel_max_arg.getValue();
		use_mouse_point = use_mouse_arg.getValue();
		std::string point_movement_string = point_movement_type_arg.getValue();
		
		if (point_movement_string == "none")
			point_movement_type = movement::none;
		else if (point_movement_string == "circle")
			point_movement_type = movement::circle;
		else if (point_movement_string == "velocity")
			point_movement_type = movement::velocites;
		else
			throw TCLAP::ArgException("Invalid movement type");
			
		sorting_algo_is_block = sorting_algo_is_block_arg.getValue();
		sorting_algo = sorting_algo_arg.getValue();
		value_algo_is_block = value_algo_is_block_arg.getValue();
		value_algo = value_algo_arg.getValue();
		rotate_colors = rotate_colors_arg.getValue();
		draw_circles = draw_circles_arg.getValue();
		ordering_greater = ordering_greater_arg.getValue();
		
		config_path = config_path_arg.getValue();
		if (! shaders_are_here(config_path)) {
			std::cout << "Shaders were not found at " <<
			          (config_path_arg.isSet() ? config_path : "XDG_CONFIG_HOME or ~/.config") << std::endl;
			std::exit(2);
		}
		
		render_to_image = image_filename_arg.isSet();
		image_filename = image_filename_arg.getValue();
		
	} catch (TCLAP::ArgException &e) {
		std::cerr << "error:" << e.error() << std::endl;
		std::exit(1);
	}

	GLFWwindow *window = initialize_glfw(window_size.x, window_size.y, !render_to_image);
	initialize_glad();
	
	Shader vertex_shader(GL_VERTEX_SHADER, "VERTEX");
	vertex_shader.read_file(config_path + vertex_filename);
	vertex_shader.compile();
	
	if (use_grid_points)
		num_points = grid_dimensions.x * grid_dimensions.y;
		
	Shader fragment_shader(GL_FRAGMENT_SHADER, "FRAGMENT");
	fragment_shader.source << "#version 460\n#define NUM_POINTS " << num_points <<
	                       "\n#define NUM_USED " << num_used << "\n";
	                       
	if (draw_circles)
		fragment_shader.source << "#define DRAW_CIRCLES\n";
	if (rotate_colors)
		fragment_shader.source << "#define ROTATE_COLORS\n";
	if (sorting_algo_is_block)
		fragment_shader.source << "#define SORT_USE_BODY\n";
	if (value_algo_is_block)
		fragment_shader.source << "#define VALUE_USE_BODY\n";
		
	fragment_shader.source << "#define VALUE_ALGO " << value_algo << std::endl;
	fragment_shader.source << "#define SORT_ALGO " << sorting_algo << std::endl;
	fragment_shader.source << "#define ORDERING " << (ordering_greater ? ">" : "<") << std::endl;
	fragment_shader.source << "#line 1\n"; // for proper error messages
	
	// appends the contents of that file to the existing contents
	fragment_shader.read_file(config_path + fragment_filename);
	fragment_shader.compile();
	
	unsigned int shaderProgram = glCreateProgram();
	
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


	GLuint fbo, render_buf;
	if (render_to_image) {
		glGenFramebuffers(1, &fbo);
		glGenRenderbuffers(1, &render_buf);
		glBindRenderbuffer(GL_RENDERBUFFER, render_buf);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_BGRA, window_size.x, window_size.y);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, render_buf);
	}
	
	// Here is where the actual program starts
	std::default_random_engine rand_engine((std::random_device()()));
	std::uniform_real_distribution<> vel_max_rand(- point_vel_max, point_vel_max);
	std::uniform_real_distribution<> rand_in_margins_x(point_margins.x, 1 - point_margins.x);
	std::uniform_real_distribution<> rand_in_margins_y(point_margins.y, 1 - point_margins.y);
	
	std::vector<vec2> points(num_points);
	std::vector<float> point_speeds(num_points);
	std::vector<vec2> effective_points(num_points);
	std::vector<vec2> point_velocities(num_points);
	std::vector<vec3> point_colors(num_points);
	
	if (use_grid_points) {
		int i = 0;
		for (int x = 0; x < grid_dimensions.x; x++) {
			for (int y = 0; y < grid_dimensions.y; y++) {
				points.at(i) = { static_cast<float>(x) / (grid_dimensions.x - 1), static_cast<float>(y) / (grid_dimensions.y - 1) };
				i++;
			}
		}
	} else {
		for (int i = 0; i < num_points; i++) {
			points[i] = { static_cast<GLfloat>(rand_in_margins_x(rand_engine)),  static_cast<GLfloat>(rand_in_margins_y(rand_engine)) };
		}
	}
	
	for (int i = 0; i < num_points; i++) {
		point_colors[i] = { // Populate with random colors
			std::uniform_real_distribution<float>(color_min.r, color_max.r)(rand_engine),
			std::uniform_real_distribution<float>(color_min.g, color_max.g)(rand_engine),
			std::uniform_real_distribution<float>(color_min.b, color_max.b)(rand_engine),
		};
	}
	
	for (auto &i : point_speeds)
		i = std::uniform_real_distribution<>(-1, 1)(rand_engine);
		
	glUseProgram(shaderProgram);
	
	int timeLocation = glGetUniformLocation(shaderProgram, "time");
	if (timeLocation == -1)
		puts("failed at time\n");
	int pointsLocation = glGetUniformLocation(shaderProgram, "points");
	if (pointsLocation == -1)
		puts("failed at points\n");
	int pointColorsLocation = glGetUniformLocation(shaderProgram, "point_colors");
	if (pointColorsLocation == -1)
		puts("failed at colors\n");
		
	double initial_time = glfwGetTime();
	unsigned int elapsed_frames = 0;
	
	if (point_movement_type == movement::velocites) {
		for (int i = 0; i < num_points; i++)
			effective_points[i] = points[i];
			
		for (int i = 0; i < num_points; i++) {
			point_velocities[i] = { static_cast<GLfloat>(vel_max_rand(rand_engine)), static_cast<GLfloat>(vel_max_rand(rand_engine))};
		}
	}
	
	if (point_movement_type == movement::none)
		effective_points = points;

	
	auto clamper = [](float in, float low, float high) {
		// returns -1 if in is less than low, 0 if its between, and 1 if its above high
		return (in > high) - (in < low);
	};
	
	cv::Mat img(window_size.x, window_size.y, CV_8UC3);
	if (render_to_image) {
		glPixelStorei(GL_PACK_ALIGNMENT, (img.step & 3) ? 1 : 4);
		glPixelStorei(GL_PACK_ROW_LENGTH, img.step / img.elemSize());
	}
	
	if (render_to_image)
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

	while (!glfwWindowShouldClose(window)) {
		if (!render_to_image)
			glfwGetWindowSize(window, &window_size.x, &window_size.y);
		float time = glfwGetTime();
		
		switch (point_movement_type) { // Here is where the points are moved according to point_movement_type
		case movement::circle:
			for (int i = 0; i < num_points; i++) { // move the points in a circle
				effective_points[i] = {
					points[i].x + std::sin(time * 1 * point_speeds[i]) / 200,
					points[i].y + std::cos(time * 1 * point_speeds[i]) / 200
				};
			}
			
		case movement::velocites:
			for (int i = 0; i < num_points; i ++) {
				effective_points[i].x += point_velocities[i].x;
				effective_points[i].y += point_velocities[i].y;
				
				point_velocities[i].x += -0.001 * clamper(effective_points[i].x, point_margins.x, 1 - point_margins.x);
				point_velocities[i].y += -0.001 * clamper(effective_points[i].y, point_margins.y, 1 - point_margins.y);
				
				point_velocities[i].x += std::sin(time + point_colors[i].r * 360) * point_vel_max / 100;
				point_velocities[i].y += std::cos(time + point_colors[i].g * 360) * point_vel_max / 100;
				
				if (point_velocities[i].x > point_vel_max) point_velocities[i].x *= 0.95;
				if (point_velocities[i].y > point_vel_max) point_velocities[i].y *= 0.95;
			}
		case movement::none:
		{} {} {} {} {} // do a whole lot of nothing
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
		glUniform1f(timeLocation, time / 8);
		
		// Do all the stuff for opengl to actually do something
		if (!render_to_image)
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
	}
	if (render_to_image) {
		cv::flip(img, img, 0);
		cv::imwrite(image_filename, img);
	}
	glfwTerminate();
	return 0;
}
