#include "material.h"
#include <iostream>

using namespace std;

int Material::textCount = 0;

Material::Material(std::string n){
	name = n;
	textName = "";
	id = 0;
}

int Material::getTextCount(void){
	return textCount;
}

void Material::setTextCount(int count){
	textCount = count;
}

std::string Material::getName(void){
	return name;
}

float* Material::getDiffuse(void){
	// cout<<"Diffuse"<<endl;
	// cout<<diffuse[0]<<endl;
	// cout<<diffuse[1]<<endl;
	// cout<<diffuse[2]<<endl;
	return diffuse;
}

float* Material::getAmbient(void){
	// cout<<"Ambient"<<endl;
	// cout<<ambient[0]<<endl;
	// cout<<ambient[1]<<endl;
	// cout<<ambient[2]<<endl;
	return ambient;
}

float* Material::getSpecular(void){
	// cout<<"Specular"<<endl;
	// cout<<specular[0]<<endl;
	// cout<<specular[1]<<endl;
	// cout<<specular[2]<<endl;
	return specular;
}

float Material::getShininess(void){
	return shininess;
}

int Material::getID(void){
	return id;
}

std::string Material::getTextName(void){
	return textName;
}

void Material::setDiffuse(float r, float g, float b){
	diffuse[0] = r;
	diffuse[1] = g;
	diffuse[2] = b;
}

void Material::setAmbient(float r, float g, float b){
	ambient[0] = r;
	ambient[1] = g;
	ambient[2] = b;
}

void Material::setSpecular(float r, float g, float b){
	specular[0] = r;
	specular[1] = g;
	specular[2] = b;
}

void Material::setShininess(float n){
	shininess = n;
}

void Material::setD(float d){
	diffuse[3] = d;
	ambient[3] = d;
	specular[3] = d;
}

void Material::setTextName(std::string tn){
	if(textName.empty()){
		textCount++;
	}
	textName = tn;
}

void Material::setID(int i){
	id = i;
}

bool Material::hasText(void){
	return !textName.empty();
}