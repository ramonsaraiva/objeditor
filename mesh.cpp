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

void Mesh::renderVerts(void)
{
    int vertex_name = 0;

    glColor3f(1.0f, 1.0f, 0.0f);
	
	for(Vertex v : verts){
				
        glLoadName(vertex_name++);
        glBegin(GL_POINTS);
		
		glVertex3fv(v.getCoords());
	
		// glPushMatrix();

			// glTranslatef(vert[0], vert[1], vert[2]);

			// glutSolidSphere(0.1, 4, 4);
				
		// glPopMatrix();
	    glEnd();
	}
}


void Mesh::render(int renderMode, int glMode){
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

void Mesh::upload_to_gpu()
{
    int it = 0;

    /*
     * array sizes
     */

    geometry_size = verts.size() * 6;
    indices_size = 0;

    /*
     * filling geometry array 
     *
     * pos 0, 1, 2 -> vertex X, Y, Z
     * pos 3, 4, 5 -> normal X, Y, Z
     *
     * if necessary will add 2 positions for the texture
     * resulting in an 8 dim. array
     */

    geometry = new GLfloat[geometry_size];

    for (unsigned int i = 0; i < verts.size(); i++)
    {
        for (int j = 0; j < 3; j++)
            geometry[it++] = (GLfloat) verts.at(i).getCoords()[j];
        for (int j = 0; j < 3; j++)
            geometry[it++] = (GLfloat) norms.at(i).getCoords()[j];
    }

    it = 0;
    
    /* 
     * filling indices array
     */

    for (Group* group : groups)
        for (Face* face : group->getFaces())
            indices_size += face->getVerts().size();

    indices = new GLuint[indices_size];

    for (Group* group : groups)
        for (Face* face : group->getFaces())
            for (int index : face->getVerts())
                indices[it++] = (GLuint) index;

    /*
     * setting up geometry buffer 
     */

    glGenBuffersARB(1, &geometry_vboid);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, geometry_vboid);
    glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(GLfloat) * geometry_size, geometry, GL_STATIC_DRAW_ARB);

    /*
     * setting indices buffer 
     */

    glGenBuffersARB(1, &indices_vboid);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, indices_vboid);
    glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(GLuint) * indices_size, indices, GL_STATIC_DRAW_ARB);

    delete [] geometry;
    delete [] indices;
}

void Mesh::render_gpu_data()
{
    glColor3f(1.0, 1.0, 1.0);
    /*
     * binding buffers with the VBO id pointer
     */

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, geometry_vboid);
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, indices_vboid);

    /*
     * enabling normals and vertices
     */ 
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);

    /* rendering */
    glNormalPointer(GL_FLOAT, sizeof(GLfloat) * 6, (float*)(sizeof(GLfloat) * 3));
    glVertexPointer(3, GL_FLOAT, sizeof(GLfloat) * 6, 0);
    glDrawElements(GL_TRIANGLES, indices_size, GL_UNSIGNED_INT, 0);

    /*
     * disabling normals and vertices
     */ 
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    /*  
     * unbinding buffers
     */
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
}

void Mesh::clear_selection() {
    selection.face = NULL;
}

bool Mesh::has_face_selected()
{
    return selection.face != NULL;
}

bool Mesh::complexify()
{
    if (!has_face_selected())
        return false;

   float xyz[3] = {0}; 

   for (int v : selection.face->getVerts())
       for (int i = 0; i < 3; i++)
           xyz[i] += verts[v].getCoords()[i] / selection.face->getVerts().size();

   Vertex centroid = Vertex(xyz);

   addVerts(centroid);

   for (unsigned int i = 0; i < selection.face->getVerts().size(); i++)
   {
       Face* f = new Face();

       f->addVert(selection.face->getVerts()[i]);
       f->addVert(selection.face->getVerts()[(i + 1) % (selection.face->getVerts().size())]);
       f->addVert(verts.size() - 1);

       groups[selection.group_pos]->addFace(f);
   }

   groups[selection.group_pos]->eraseFaceAt(selection.face_pos);
   clear_selection();

   return true;
}

void Mesh::random_complexify()
{
    int random_group_index = rand_lim(groups.size() - 1);
    int random_face_index = rand_lim(groups[random_group_index]->getFaces().size() - 1);

    selection.group_pos = random_group_index;
    selection.face_pos = random_face_index;
    selection.face = groups[random_group_index]->getFaceAt(random_face_index);

    complexify();
}

void Mesh::triangulate()
{
    stack<Mesh::FaceSel> not_a_triangle;

    for (unsigned int i = 0; i < groups.size(); i++)
    {
        for (unsigned int j = 0; j < groups[i]->getFaces().size(); j++)
        {
            Face* curr_face = groups[i]->getFaceAt(j);

            if (curr_face->getVerts().size() == 3)
                continue;

            Mesh::FaceSel sel;

            sel.group_pos = i;
            sel.face_pos = j;
            sel.face = curr_face;

            not_a_triangle.push(sel); 
        }
    }

    while (!not_a_triangle.empty())
    {
        Mesh::FaceSel nat = not_a_triangle.top();

        selection.group_pos = nat.group_pos;
        selection.face_pos = nat.face_pos;
        selection.face = nat.face;

        not_a_triangle.pop();

        complexify();
    }
}

void Mesh::render_new_face(float* xyz) {
    Vertex new_vertex = Vertex(xyz);
    int vertex_pos_first = 0;
    int vertex_pos_second = 1;
    int group_pos = 0;

    float min_dist = distance_bet(new_vertex, verts[vertex_pos_first]);  

    for (unsigned int i = 0; i < groups.size(); i++)
    {
        for (unsigned int j = 0; j < groups[i]->getFaces().size(); j++)
        {
            Face* curr_face = groups[i]->getFaces()[j];

            for (int vertex : curr_face->getVerts())
            {
                float dist = distance_bet(new_vertex, verts[vertex]);

                if (dist < min_dist)
                {
                    vertex_pos_second = vertex_pos_first;

                    min_dist = dist;
                    vertex_pos_first = vertex;

                    group_pos = i;
                }
            }
        }
    }

    addVerts(new_vertex); 

    Face* new_face = new Face();

    new_face->addVert(verts.size() - 1);
    new_face->addVert(vertex_pos_first);
    new_face->addVert(vertex_pos_second);

    groups[group_pos]->addFace(new_face); 
}

void Mesh::mess()
{
   random_shuffle(verts.begin(), verts.end());
}

int Mesh::rand_lim(int limit)
{
    int divisor = RAND_MAX / (limit + 1);
    int retval;

    do
        retval = rand() / divisor;
    while (retval > limit);

    return retval;
}

float Mesh::distance_bet(Vertex v1, Vertex v2)
{
    float* v1xyz = v1.getCoords();
    float* v2xyz = v2.getCoords();

    return sqrt(pow(v2xyz[0] - v1xyz[0], 2) + pow(v2xyz[1] - v1xyz[1], 2) + pow(v2xyz[2] - v2xyz[1], 2));
}
