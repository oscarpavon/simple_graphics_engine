CC=g++ -g -std=c++17
Library=-lglfw -lpthread 

MAIN_OBJS = main.cpp 3D_objects.cpp camera.cpp engine.cpp

.ONESHELL:
main:
	mkdir -p bin && cd src
	$(CC) $(MAIN_OBJS) renderer.cpp -o ../renderer $(Library) -I./ -lvulkan 

renderer.o:
	$(CC) -c renderer.cpp -I./

.ONESHELL:
gl:
	mkdir -p bin && cd src
	$(CC) $(MAIN_OBJS) opengl_renderer.cpp -o ../renderer $(Library) -lGLEW -lGL -I./ -D_OpenGL_Renderer_
