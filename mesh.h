#include "group.h"
#include "vertex.h"
#include "texts.h"
#include "material.h"
#include <iostream>
#include <map>
#include <algorithm>
#include <GL/glew.h>
#include <GL/glut.h>

using namespace std;

class Mesh{
    public:
        struct FaceSel
        {
            int group_pos;
            int face_pos;
            Face* face = NULL;
        };

	private:
		vector<Group*> groups;
		vector<Vertex> verts;
		vector<Vertex> norms;
		vector<Texts> texts;
		map<string, Material*> mats;
        struct FaceSel selection;

        GLfloat* geometry;
        GLuint* indices;

        int geometry_size;
        int indices_size;

        GLuint geometry_vboid;
        GLuint indices_vboid;

	public:
		void addGroup(Group* newGroup);
		void addVerts(Vertex newV);
		void addNorms(Vertex newN);
		void addTexts(Texts newT);
		void addMats(Material* newM);

		void render(int renderMode, int glMode);
		void renderVerts(void);

        void upload_to_gpu();
        void render_gpu_data();

        void set_selection(int group, int face);
        void clear_selection();
        bool has_face_selected();
        bool complexify();
        void random_complexify();

        void mess();
        int rand_lim(int limit);


		Group* getGroupAt(int i);
		vector<Group*> getGroups(void);
		vector<Vertex> getVerts(void);
		vector<Vertex> getNorms(void);
		vector<Texts> getTexts(void);
		map<string, Material*> getMats(void);
		Material* getMtl(string name);
        FaceSel* get_selection();
};
