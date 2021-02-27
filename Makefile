SOURCE := main.cpp
TARGET := main

all: build run clean

install:
	#  https://formulae.brew.sh/formula/freeglut
	# https://cs.lmu.edu/~ray/notes/opengl/
	brew install freeglut

build:
	g++ $(SOURCE) -framework OpenGL -framework GLUT -o $(TARGET)

clean:
	rm $(TARGET)

run:
	./$(TARGET)
