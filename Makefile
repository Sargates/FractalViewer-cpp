all:
	g++ -std=c++20 -o main main.cpp `sdl2-config --cflags --libs`
	./main

build:
	g++ -std=c++20 -o main main.cpp `sdl2-config --cflags --libs`
