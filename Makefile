CXX = g++
CFLAGS = -Wall
LDFLAGS = -std=c++0x

LIBS = -lGL -lglut -lGLU

run: compile
	./main.exe

compile:
	$(CXX) $(CFLAGS) -o main.exe *.cpp $(LDFLAGS) $(LIBS)
