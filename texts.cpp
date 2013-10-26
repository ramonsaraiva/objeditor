#include "texts.h"

Texts::Texts(float c[2]){
	coords[0] = c[0];
	coords[1] = c[1];
}

Texts::Texts(float x, float y){
	coords[0] = x;
	coords[1] = y;
}

float* Texts::getCoords(void){
	return coords;
}
