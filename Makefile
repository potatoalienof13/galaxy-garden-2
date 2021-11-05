all:
	g++ -Iinclude -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl main.cpp glad.c -O2 -o gg2

debug:
	g++ -Iinclude -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl main.cpp glad.c -Og -g -o gg2
