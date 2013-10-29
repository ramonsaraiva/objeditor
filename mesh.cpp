#include "mesh.h" 

void Mesh::addGroup(Group* newGroup){
	groups.push_back(newGroup);
}

void Mesh::addVerts(Vertex newV){
	verts.push_back(newV);
}

void Mesh::addNorms(Vertex newN){
	norms.push_back(newN);
}

void Mesh::addTexts(Texts newT){
	texts.push_back(newT);
}

void Mesh::addMats(Material* newM){
	string name = newM->getName();
	if(!name.empty()){
		mats[name] = newM;
	}
}

void Mesh::set_selection(int group, int face)
{
    selection.group_pos = group;
    selection.face_pos = face;
    selection.face = groups.at(group)->getFaces().at(face); 
}

vector<Group*> Mesh::getGroups(void){
	return groups;
}

Group* Mesh::getGroupAt(int i){
	return groups[i];
}

vector<Vertex> Mesh::getVerts(void){
	return verts;
}

vector<Vertex> Mesh::getNorms(void){
	return norms;
}

vector<Texts> Mesh::getTexts(void){
	return texts;
}

map<string, Material*> Mesh::getMats(void){
	return mats;
}

Material* Mesh::getMtl(string name){
	return mats[name];
}

Mesh::FaceSel* Mesh::get_selection()
{
    return &selection;
}

void Mesh::render(int renderMode){
	int group_name = 0;
	int currentID = 0;
	
	glBindTexture(GL_TEXTURE_2D, currentID);
	
	for(Group* g : groups){	
	
		if(renderMode == GL_SELECT){
			glLoadName(group_name++);
		}
		
		if(!g->getVisible()){
			continue;
		}
		
		string mtlName = g->getMtl();
		
		if(!mtlName.empty()){
		
			Material* mtl = getMtl(mtlName);
			glMaterialfv(GL_FRONT, GL_SPECULAR, mtl->getSpecular());
			glMaterialfv(GL_FRONT, GL_AMBIENT, mtl->getAmbient());
			glMaterialfv(GL_FRONT, GL_DIFFUSE, mtl->getDiffuse());
			glMaterialf(GL_FRONT, GL_SHININESS, mtl->getShininess());
			
			int tID = mtl->getID();
			if(tID != currentID){
				currentID = tID;
				glBindTexture(GL_TEXTURE_2D, currentID);
			}
		}	
		
		int face_name = 0;
		
		for(Face* f : g->getFaces()){

			vector<int> v = f->getVerts();
			vector<int> n = f->getNorms();
			vector<int> t = f->getTexts();
			
			bool hasNorm = !n.empty();
			bool hasText = !t.empty();
			
			int nv = v.size();
			
			if(renderMode == GL_SELECT){
				glPushName(face_name++);
			}
		
			glBegin(GL_POLYGON);
			
			for(int x = 0; x < nv; ++x){
				if(hasNorm) {
					glNormal3fv(norms[n[x]].getCoords());
				}
				if(hasText){
					glTexCoord2fv(texts[t[x]].getCoords());
				}
				glVertex3fv(verts[v[x]].getCoords());
			}
			
			glEnd();
			
			if(renderMode == GL_SELECT){
				glPopName();
			}
			
		}
	}
}

void Mesh::renderVerts(void){
	
	glBegin(GL_POINTS);
	
	for(Vertex v : verts){
				
		glColor3f(1.0f, 1.0f, 0.0f);
		
		glVertex3fv(v.getCoords());
	
		// glPushMatrix();

			// glTranslatef(vert[0], vert[1], vert[2]);

			// glutSolidSphere(0.1, 4, 4);
				
		// glPopMatrix();
	}
	glEnd();
}


void Mesh::render2(int renderMode, int glMode){
	int group_name = 0;
	int currentID = 0;
	
	glBindTexture(GL_TEXTURE_2D, currentID);
	
	for(Group* g : groups){	
	
		if(renderMode == GL_SELECT){
			glLoadName(group_name++);
		}
		
		if(!g->getVisible()){
			continue;
		}
		
		string mtlName = g->getMtl();
		
		if(!mtlName.empty()){
		
			Material* mtl = getMtl(mtlName);
			glMaterialfv(GL_FRONT, GL_SPECULAR, mtl->getSpecular());
			glMaterialfv(GL_FRONT, GL_AMBIENT, mtl->getAmbient());
			glMaterialfv(GL_FRONT, GL_DIFFUSE, mtl->getDiffuse());
			glMaterialf(GL_FRONT, GL_SHININESS, mtl->getShininess());
			
			int tID = mtl->getID();
			if(tID != currentID){
				currentID = tID;
				glBindTexture(GL_TEXTURE_2D, currentID);
			}
		}	
		
		int face_name = 0;
		
		for(Face* f : g->getFaces()){
			if (f == selection.face)
                glColor3f(0.603922f, 0.803922f, 0.196078f);
            else
                glColor3f(1.0, 1.0, 1.0);
			
			vector<int> v = f->getVerts();
			vector<int> n = f->getNorms();
			vector<int> t = f->getTexts();
			
			bool hasNorm = !n.empty();
			bool hasText = !t.empty();
			
			int nv = v.size();
			
			if(renderMode == GL_SELECT){
				glPushName(face_name++);
			}
		
			glBegin(glMode);
			
			for(int x = 0; x < nv; ++x){
				if(hasNorm) {
					glNormal3fv(norms[n[x]].getCoords());
				}
				if(hasText){
					glTexCoord2fv(texts[t[x]].getCoords());
				}
				glVertex3fv(verts[v[x]].getCoords());
			}
			
			glEnd();
			
			if(renderMode == GL_SELECT){
				glPopName();
			}
			
		}
	}
	
	if(glMode == GL_LINE_LOOP){
		renderVerts();
	}
}

void Mesh::holytest()
{
    int it = 0;
    vertices_size = verts.size() * 3;

    indices_size = 0;
    vertices = new GLfloat[vertices_size];
    
    for (Vertex v : verts)
    {
        for (int i = 0; i < 3; i++)
            vertices[it++] = (GLfloat) v.getCoords()[i];
    }

    it = 0;

    for (Group* group : groups)
        for (Face* face : group->getFaces())
            indices_size += face->getVerts().size();

    indices = new GLuint[indices_size];

    for (Group* group : groups)
        for (Face* face : group->getFaces())
            for (int index : face->getVerts())
                indices[it++] = (GLuint) index;

    glGenBuffersARB(1, &vertices_vboid);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, vertices_vboid);
    glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(GLfloat) * vertices_size, vertices, GL_STATIC_DRAW_ARB);

    glGenBuffersARB(1, &indices_vboid);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, indices_vboid);
    glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(GLuint) * indices_size, indices, GL_STATIC_DRAW_ARB);
}

void Mesh::holytest2()
{
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, vertices_vboid);
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, indices_vboid);

    glEnableClientState(GL_VERTEX_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, 0);
    glDrawElements(GL_TRIANGLES, indices_size, GL_UNSIGNED_INT, 0);

    glDisableClientState(GL_VERTEX_ARRAY);

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
}

void Mesh::mess()
{
   random_shuffle(verts.begin(), verts.end());
}
