#include "group.h"
#include "vertex.h"
#include "texts.h"
#include "material.h"
#include <iostream>
#include <map>
#include <GL/gl.h> 
#include <GL/glut.h>

using namespace std;

class Mesh{
	private:
		vector<Group*> groups;
		vector<Vertex> verts;
		vector<Vertex> norms;
		vector<Texts> texts;
		map<string, Material*> mats;
        Face* selection;

	public:
		void addGroup(Group* newGroup);
		void addVerts(Vertex newV);
		void addNorms(Vertex newN);
		void addTexts(Texts newT);
		void addMats(Material* newM);
		void render(int renderMode);
		void render2(int renderMode, int glMode);
		void renderVerts(void);
        void set_selection(int group, int face);

		Group* getGroupAt(int i);
		vector<Group*> getGroups(void);
		vector<Vertex> getVerts(void);
		vector<Vertex> getNorms(void);
		vector<Texts> getTexts(void);
		map<string, Material*> getMats(void);
		Material* getMtl(string name);
        Face* get_selection();
};
