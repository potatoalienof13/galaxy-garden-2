all:
	c++ -Iinclude -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl main.cpp shader.cpp initialization.cpp glad.cpp -O2 -o gg2

debug:
	c++ -Iinclude -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl main.cpp shader.cpp initialization.cpp glad.cpp -Og -g -o gg2
