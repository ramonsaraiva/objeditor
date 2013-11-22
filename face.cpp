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


bool Face::has_vertex(int v)
{
	bool has = false;

	for (int i = verts.size() - 1; i >= 0; i--)
	{
		if (verts[i] > v)
			verts[i]--;
		else if (verts[i] == v)
		{
			verts.erase(verts.begin() + i);
			has = true;
		}
	}

	return has;
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
