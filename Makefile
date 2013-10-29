CXX = g++
CFLAGS = -Wall
LDFLAGS = -std=c++0x

	LIBS = -lGL -lGLEW -lglut -lGLU

run: compile
	./main.exe

compile:
	$(CXX) $(CFLAGS) -o main.exe *.cpp $(LDFLAGS) $(LIBS)
