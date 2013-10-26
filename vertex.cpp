#include "vertex.h"

Vertex::Vertex(float c[3]){
	coords[0] = c[0];
	coords[1] = c[1];
	coords[2] = c[2];
}

Vertex::Vertex(float x, float y, float z){
	coords[0] = x;
	coords[1] = y;
	coords[2] = z;
}

float* Vertex::getCoords(void){
	return coords;
}
