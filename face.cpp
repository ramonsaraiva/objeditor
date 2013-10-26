#include "face.h"

void Face::addVert(int v){
	verts.push_back(v);
}

void Face::addNorm(int n){
	norms.push_back(n);
}

void Face::addText(int t){
	texts.push_back(t);
}

vector<int> Face::getVerts(void){
	return verts;
}

vector<int> Face::getNorms(void){
	return norms;
}

vector<int> Face::getTexts(void){
	return texts;
}
