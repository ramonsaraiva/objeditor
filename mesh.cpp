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
			if (f == selection)
                glColor3f(1.0, 0.0, 1.0);
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

void Mesh::set_selection(int group, int face)
{
    selection = groups.at(group)->getFaces().at(face);
}

Face* Mesh::get_selection()
{
    return selection;
}
