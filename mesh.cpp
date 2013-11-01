#include "mesh.h" 

void Mesh::set_name(string n)
{
    name = n;
}

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

void Mesh::set_face_selected(int group, int face)
{
    face_selected.group_pos = group;
    face_selected.face_pos = face;
    face_selected.face = groups.at(group)->getFaces().at(face); 

    selection = SELECTION_FACE;
}

void Mesh::set_vertex_selected(int vertex)
{
    vertex_selected = vertex;
    selection = SELECTION_VERTEX;
}

string Mesh::get_name()
{
    return name;
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

Mesh::FaceSel* Mesh::get_face_selected()
{
    return &face_selected;
}

float* Mesh::get_selected_vertex_xyz()
{
    return verts[vertex_selected].getCoords();
}

void Mesh::renderVerts(void)
{
    int vertex_name = 0;

    glColor3f(1.0f, 1.0f, 0.0f);
	
	for (unsigned int i = 0; i < verts.size(); i++)
    {
        if (selection == SELECTION_VERTEX && (int) i == vertex_selected)
            glColor3f(1.0f, 0.0f, 1.0f);
				
        glLoadName(vertex_name++);

        glBegin(GL_POINTS);
		glVertex3fv(verts[i].getCoords());
	    glEnd();

        if (selection == SELECTION_VERTEX && (int) i == vertex_selected)
            glColor3f(1.0f, 1.0f, 0.0f);
	}
}

void Mesh::render(int renderMode, int glMode){
	int group_name = 0;
	int currentID = 0;
	
	glBindTexture(GL_TEXTURE_2D, currentID);

	if (glMode == GL_LINE_LOOP)
    {
		renderVerts();

        if (renderMode == GL_SELECT) return;
	}
	
	for(Group* g : groups){	
	
		if(renderMode == GL_SELECT && glMode == GL_POLYGON){
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

        glColor3f(1.0, 1.0, 1.0);

		for(Face* f : g->getFaces()){
			if (f == face_selected.face)
                glColor3f(0.603922f, 0.803922f, 0.196078f);
			
			vector<int> v = f->getVerts();
			vector<int> n = f->getNorms();
			vector<int> t = f->getTexts();
			
			bool hasNorm = !n.empty();
			bool hasText = !t.empty();
			
			int nv = v.size();
			
			if (renderMode == GL_SELECT && glMode == GL_POLYGON)
				glPushName(face_name++);
		
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
			
			if (renderMode == GL_SELECT && glMode == GL_POLYGON)
				glPopName();

            if (f == face_selected.face)
                glColor3f(1.0, 1.0, 1.0);
		}
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

void Mesh::clear_selection()
{
    selection = SELECTION_NONE;

    face_selected.group_pos = -1;
    face_selected.face_pos = -1;
    face_selected.face = NULL;

    vertex_selected = -1;
}

int Mesh::selection_type()
{
    return selection;
}

void Mesh::delete_selected_face()
{
    groups[face_selected.group_pos]->eraseFaceAt(face_selected.face_pos);
    clear_selection();
}

bool Mesh::complexify()
{
    if (selection != SELECTION_FACE)
        return false;

   float xyz[3] = {0}; 

   for (int v : face_selected.face->getVerts())
       for (int i = 0; i < 3; i++)
           xyz[i] += verts[v].getCoords()[i] / face_selected.face->getVerts().size();

   Vertex centroid = Vertex(xyz);

   addVerts(centroid);

   for (unsigned int i = 0; i < face_selected.face->getVerts().size(); i++)
   {
       Face* f = new Face();

       f->addVert(face_selected.face->getVerts()[i]);
       f->addVert(face_selected.face->getVerts()[(i + 1) % (face_selected.face->getVerts().size())]);
       f->addVert(verts.size() - 1);

       groups[face_selected.group_pos]->addFace(f);
   }

   groups[face_selected.group_pos]->eraseFaceAt(face_selected.face_pos);
   clear_selection();

   return true;
}

void Mesh::random_complexify()
{
    int random_group_index = rand_lim(groups.size() - 1);
    int random_face_index = rand_lim(groups[random_group_index]->getFaces().size() - 1);

    face_selected.group_pos = random_group_index;
    face_selected.face_pos = random_face_index;
    face_selected.face = groups[random_group_index]->getFaceAt(random_face_index);
    selection = SELECTION_FACE;

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

        face_selected.group_pos = nat.group_pos;
        face_selected.face_pos = nat.face_pos;
        face_selected.face = nat.face;
        selection = SELECTION_FACE;

        not_a_triangle.pop();

        complexify();
    }
}

void Mesh::render_new_face(float* xyz) {
    Vertex new_vertex = Vertex(xyz);
    unsigned int vertex_pos_first = 0;
    unsigned int vertex_pos_second;

    float min_dist = distance_bet(new_vertex, verts[vertex_pos_first]);  

    for (unsigned int i = 1; i < verts.size(); i++)
    {
        float dist = distance_bet(new_vertex, verts[i]);

        if (dist < min_dist)
        {
            min_dist = dist;
            vertex_pos_first = i;
        }
    }

    do
        vertex_pos_second = rand_lim(verts.size());
    while (vertex_pos_second == vertex_pos_first);

    min_dist = distance_bet(new_vertex, verts[vertex_pos_second]);

    for (unsigned int i = 0; i < verts.size(); i++)
    {
        float dist = distance_bet(new_vertex, verts[i]);

        if (dist < min_dist && i != vertex_pos_first)
        {
            min_dist = dist;
            vertex_pos_second = i;
        }
    }

    addVerts(new_vertex); 

    Face* new_face = new Face();

    new_face->addVert(verts.size() - 1);
    new_face->addVert(vertex_pos_first);
    new_face->addVert(vertex_pos_second);

    groups[groups.size() - 1]->addFace(new_face); 
}

void Mesh::move_selected_vertex(int side)
{
    switch (side)
    {
        case MOVE_VERTEX_X_POS:
            verts[vertex_selected].getCoords()[0] += VERTEX_MOV;
            break;
        case MOVE_VERTEX_X_NEG:
            verts[vertex_selected].getCoords()[0] -= VERTEX_MOV;
            break;
        case MOVE_VERTEX_Y_POS:
            verts[vertex_selected].getCoords()[1] += VERTEX_MOV;
            break;
        case MOVE_VERTEX_Y_NEG:
            verts[vertex_selected].getCoords()[1] -= VERTEX_MOV;
            break;
        case MOVE_VERTEX_Z_POS:
            verts[vertex_selected].getCoords()[2] += VERTEX_MOV;
            break;
        case MOVE_VERTEX_Z_NEG:
            verts[vertex_selected].getCoords()[2] -= VERTEX_MOV;
            break;
    }
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
