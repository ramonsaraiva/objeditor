#include "group.h"

Group::Group(){
	name = "";
	visible = true;
}

Group::Group(string n){
	name = n;
	visible = true;
}

vector<Face*> Group::getFaces(void){
	return faces;
}

string Group::getName(void){
	return name;
}

string Group::getMtl(void){
	return mtl;
}

void Group::eraseFaceAt(int i){
	faces.erase(faces.begin() + i);
}

bool Group::getVisible(void){
	return visible;
}

void Group::addFace(Face* newFace){
	faces.push_back(newFace);
}

void Group::setName(string n){
	name = n;
}

void Group::setMtl(string name){
	mtl = name;
}

void Group::setVisible(bool v){
	visible = v;
}
