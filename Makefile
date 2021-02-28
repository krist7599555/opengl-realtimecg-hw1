SOURCE := main.cpp
TARGET := main
# ARGS := -ka 0 0 0 -kd 0.55231 0.232 0.5612 -sp 30 -ks 0.262344 0.623421 0.1233 -dl -0.5 0.1234 -0.42 0.235 0.263 0.5 -pl 1.512 1.123 1.132 0.125 0.643 0.6423 -dl 0.125 0.125 0.125 0.125 0.125 0.125 -dl 0.6423 0.346 0.634 0.124 0.623 0.623 -dl 0.12312 0.512 0.1231 0.1231 0.123 0.5612 -dl 0.125 0.1245 0.321 1 1 1 -pl 1.512 1.3123 1.312 0.123 0.6236 0.623 -pl 1.1236 1.1236 1.6234 0.23460 0.3462 0.2346 -pl 1.2346 1.6342 1.423 0.123 0.632 0.6432 -pl 1.1236 1.6423 1.6423 0.123 0.632 0.6432
ARGS := -ks 0.5 0.5 0.5 -kd 0.5 0.5 0.5 -dl -1 -1 -1 1 1 1

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
	./$(TARGET) $(ARGS)

dev:
	bash ./dev.sh
