all:
	g++ -Iinclude -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl main.c glad.c -O2 -o opengl-bgg

debug:
	g++ -Iinclude -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl main.c glad.c -Og -g -o opengl-bgg
