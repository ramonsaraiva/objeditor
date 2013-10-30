#include "camera.h"
#include "reader.h"
#include <stdio.h> 
#include <string.h>
#include <utility>
#include <algorithm>
#include <map>
#include <time.h>

#define BUFSIZE 512

#define MODE_FACE       0
#define MODE_VERTEX     1

#define RENDER_NORMAL   0
#define RENDER_VBO      1

#define CMD_EXIT             0
#define CMD_COMMANDS         6
#define CMD_DATA_SHOW        7
#define CMD_DATA_HIDE        8
#define CMD_FPS_SHOW         1
#define CMD_FPS_HIDE         2
#define CMD_OBJ_OPEN         4
#define CMD_MODE             5
#define CMD_DELETE           9
#define CMD_MESS            10
#define CMD_PLANE_SHOW      11
#define CMD_PLANE_HIDE      12
#define CMD_PLANE_SIZE      13
#define CMD_RENDER_NORMAL   14
#define CMD_RENDER_VBO      15

using namespace std;

int width = 1000;
int height = 1000;

int glMode = GL_POLYGON;

Mesh* mesh;
Camera* camera;

int render_mode = RENDER_NORMAL;

int currentTime, frames = 0, timebase;

char s[20];

unsigned int* ids;

bool terminal_mode = false;
bool fps_enabled = true;
bool data_enabled = true;
bool cartesian_plane_enabled = false;

float cartesian_plane_size = 9;

string terminal_buff;
string comp_buff;
string objfile_buff;
string viewmode_buff;
string selection_buff;

map<string, int> terminal_cmds;
vector<string> obj_files;

static GLfloat COLOR_ERROR[] = {1.0f, 0.388235f, 0.278431f};
static GLfloat COLOR_INFO[] = {0.690196f, 0.768627f, 0.870588f};
GLfloat* comp_color = COLOR_ERROR;

void init();
void drawScene();
void idle();
void calculeFps();

void handleResize(int w, int h);
void handleKeypress(unsigned char key, int x, int y);
void handleMouse(int button, int state, int x, int y);
void handlePassiveMotion(int x, int y);
void handleTerminal();
void handleAutohelper();
void handleAutocomplete();

bool loadOBJ(const char* s);

void switch2D(void);
void display2d(const char* msg, float x, float y, float l_size, void* font);

int switchSelect(GLuint* selectBuf, int x, int y);
void processHits (GLint hits, GLuint buffer[]);

void toggle_terminal();
void set_mode(int mode);
bool has_face_selected();
bool delete_selection();
bool complexify();
void random_complexify();
bool simplify();

void selection_info(int mode);
void draw_cartesian_plane();

int rand_lim(int limit);

int main(int argc, char** argv)
{
    srand(time(NULL));

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(width, height);

    glutCreateWindow("objeditor ~ marcel & ramon");
    glutFullScreen();

    glewInit();
    init();

    glutDisplayFunc(drawScene);
    glutMouseFunc(handleMouse);
    glutReshapeFunc(handleResize);
    glutKeyboardFunc(handleKeypress);
    glutPassiveMotionFunc(handlePassiveMotion);
    glutIdleFunc(idle);

    glutMainLoop();

    return 0;
}

void init()
{
    glClearColor(0.1, 0.1, 0.1, 1.0);
    glClearDepth(1.0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);
    // glEnable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_COLOR_MATERIAL);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glCullFace(GL_BACK);

    glLineWidth(3);
    glPointSize(10);

    glutSetCursor(GLUT_CURSOR_CROSSHAIR);

    GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };
    GLfloat light_specular[] = {1.0, 1.0, 1.0,1.0};
    GLfloat light_diffuse[] = {1.0, 1.0, 1.0,1.0};

    glShadeModel (GL_SMOOTH);

    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);

    camera = new Camera(90);
    camera->resetView(width, height);

    loadOBJ("obj/cube.obj");

    timebase = glutGet(GLUT_ELAPSED_TIME);

    terminal_cmds["exit"] = CMD_EXIT;
    terminal_cmds["commands"] = CMD_COMMANDS;
    terminal_cmds["data-show"] = CMD_DATA_SHOW;
    terminal_cmds["data-hide"] = CMD_DATA_HIDE;
    terminal_cmds["fps-show"] = CMD_FPS_SHOW;
    terminal_cmds["fps-hide"] = CMD_FPS_HIDE;
    terminal_cmds["obj-open"] = CMD_OBJ_OPEN;
    terminal_cmds["mode"] = CMD_MODE;
    terminal_cmds["delete"] = CMD_DELETE;
    terminal_cmds["mess"] = CMD_MESS;
    terminal_cmds["cplane-show"] = CMD_PLANE_SHOW;
    terminal_cmds["cplane-hide"] = CMD_PLANE_HIDE;
    terminal_cmds["cplane-size"] = CMD_PLANE_SIZE;
    terminal_cmds["render-normal"] = CMD_RENDER_NORMAL;
    terminal_cmds["render-vbo"] = CMD_RENDER_VBO;

    objfile_buff = "cube.obj";
    set_mode(MODE_FACE);

    obj_files = {"cat", "cone", "cow", "cube", "horse", "parthenon", "pyramid", "rabbit", "sphere", "teapot", "torreDiPisa", "whale"};
}

void drawScene()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    	
    if (render_mode == RENDER_NORMAL)
        mesh->render(GL_RENDER, glMode);
    else if (render_mode == RENDER_VBO)
        mesh->render_gpu_data();

    if (cartesian_plane_enabled)
        draw_cartesian_plane();
	
	switch2D();
	//Draw 2D stuff
    
	if (terminal_mode)
    {
        /* comp */
        glColor3fv(comp_color);
        display2d(comp_buff.c_str(), 200, 28, 14, GLUT_BITMAP_HELVETICA_18);

        /* terminal prefix */
        glColor3f(0.180392f, 0.545098f, 0.341176f);
        display2d("~$", 185, 8, 14, GLUT_BITMAP_HELVETICA_18);

        /* terminal */
		glColor3f(0.941176f, 0.901961f, 0.54902f);
        display2d(terminal_buff.c_str(), 220, 8, 14, GLUT_BITMAP_HELVETICA_18);
	}

    if (data_enabled)
    {
        if (fps_enabled)
        {
            /* fps */
            currentTime = glutGet(GLUT_ELAPSED_TIME);
            glColor3f(1.0f, 0.85f, 0.95f);
            sprintf(s,"FPS: %4.2f", frames*1000.0/(currentTime-timebase));
            display2d(s, 10, 8, 14, GLUT_BITMAP_HELVETICA_18);
        }

        /* obj file */
        glColor3f(0.180392f, 0.545098f, 0.341176f);
        display2d(objfile_buff.c_str(), 10, height - 26, 14, GLUT_BITMAP_HELVETICA_18);

        /* view mode */
        glColor3f(0.254902f, 0.411765f, 0.882353f);
        display2d(viewmode_buff.c_str(), 10, height - 49, 14, GLUT_BITMAP_HELVETICA_18);

        /* selection info */
        glColor3f(0.980392f, 0.501961f, 0.447059f);
        display2d(selection_buff.c_str(), 10, height - 72, 14, GLUT_BITMAP_HELVETICA_18);

        if (cartesian_plane_enabled)
        {
            glColor3f(1.0f, 0.0f, 0.0f);
            display2d("X", 10, 30, 14, GLUT_BITMAP_HELVETICA_18);

            glColor3f(0.0f, 1.0f, 0.0f);
            display2d("Y", 28, 30, 14, GLUT_BITMAP_HELVETICA_18);

            glColor3f(0.0f, 0.0f, 1.0f);
            display2d("Z", 45, 30, 14, GLUT_BITMAP_HELVETICA_18);
        }
    }

	//Go back to 3D
	camera->resetView(width, height);
	glEnable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	
    //Buffer swap
	glutSwapBuffers();
    glFlush();
}

void idle()
{
	currentTime = glutGet(GLUT_ELAPSED_TIME);
	calculeFps();	
	glutPostRedisplay();
}

void calculeFps()
{
	frames++;

	if (currentTime - timebase > 1000)
    {
		timebase = currentTime;
		frames = 0;
	}
}

void handleResize(int w, int h)
{
	if (h < 1) h = 1;
	width  = w;
	height = h;

	glutWarpPointer(width/2, height/2);

	// reestabele a visualização
	camera->resetView(w, h);
}

void handleKeypress(unsigned char key, int x, int y)
{
    /* esc */
    if(key == 27)
    {
        toggle_terminal();
        return;
    }

    if(terminal_mode)
    {
        switch(key)
        {
            /* backspace */
            case 8:
                if(terminal_buff.size())
                {
                    terminal_buff.pop_back();
                    if (terminal_buff.size())
                        handleAutohelper();
                    else
                        comp_buff.clear();
                }
                break;
            /* enter */
            case 13:
                handleTerminal();
                break;
            /* tab */
            case 9:
                handleAutocomplete();
                handleAutohelper();
                break;
            default:
                terminal_buff += key;
                handleAutohelper();
        }
    }
    else
    {
        switch(key)
        {
            case 'a':
            case 'A':
                camera->moveSide(1);
                break;
            case 's':
            case 'S':
                camera->move(-1);
                break;
            case 'd':
            case 'D':
                camera->moveSide(-1);
                break;
            case 'w':
            case 'W':
                camera->move(1);
                break;
            case 'h':
            case 'H':
                camera->changeAngle(-0.4);
                break;
            case 'l':
            case 'L':
                camera->changeAngle(0.4);
                break;
            case 'k':
            case 'K':
                camera->setDirectionY(0.01);
                break;
            case 'j':
            case 'J':
                camera->setDirectionY(-0.01);
                break;
            case 'f':
            case 'F':
                set_mode(MODE_FACE);
                break;
            case 'v':
            case 'V':
                set_mode(MODE_VERTEX);
                break;
            case 'x':
            case 'X':
                delete_selection();
                break;
            case 'z':
            case 'Z':
                break;
            case 'c':
            case 'C':
                complexify();
                break;
            case 'r':
            case 'R':
                random_complexify();
                break;
            case 'm':
            case 'M':
                mesh->mess(); 
                break;
            case 'p':
            case 'P':
                cartesian_plane_enabled = !cartesian_plane_enabled;
                break;
            case 'i':
            case 'I':
                camera->moveSide(1);
                camera->changeAngle(3.5);
                break;
            case 'o':
            case 'O':
                camera->moveSide(-1);
                camera->changeAngle(-3.5);
                break;
            case 'q':
            case 'Q':
                exit(0);
                break;
        }
        glutPostRedisplay();
    }
}

void handleMouse(int button, int state, int x, int y)
{
    if (state != GLUT_DOWN)
        return;

    GLuint selectBuf[BUFSIZE];
    GLint hits;

    hits = switchSelect(selectBuf, x, y);

	if (button == GLUT_LEFT_BUTTON)
    {
        if (hits != 0)
            processHits(hits,selectBuf);
    }
    
	glutPostRedisplay();
}

void handlePassiveMotion(int x, int y)
{
	float y2 = (height - y) / (float)height;
	if (y2 != 0.5 || x != width/2.0)
	{
		if(y2 != 0.5)
		{
			camera->setDirectionY(y2 - 0.5);
		}
		if(x != width/2.0)
		{
			camera->changeAngle((x - width/2.0) / 10.0);
		}
		
		glutWarpPointer(width/2.0, height/2.0);
		glutPostRedisplay();
	}
}

void handleTerminal()
{
    if (!terminal_buff.size())
    {
        toggle_terminal();
        return;
    }

    vector<string> tokens = StringHelper::split(terminal_buff, ' ', true);

    comp_buff.clear();

    if (!(terminal_cmds.find(tokens.front()) != terminal_cmds.end()))
    {
        comp_color = COLOR_ERROR;
        comp_buff = "The command \"" + tokens.front() + "\" was not found, try \"commands\" to see which is available ";
        return;
    }
    else
    {
        switch (terminal_cmds[tokens.front()])
        {
            case CMD_EXIT:
                exit(0);
                break;
            case CMD_COMMANDS:
                comp_color = COLOR_INFO;
                comp_buff.clear();
                for (auto cmd : terminal_cmds)
                {
                    if (cmd.first != "commands")
                        comp_buff += cmd.first + " ";
                }
                break;
            case CMD_DATA_SHOW:
                data_enabled = true;
                break;
            case CMD_DATA_HIDE:
                data_enabled = false;
                break;
            case CMD_FPS_SHOW:
                fps_enabled = true;
                break;
            case CMD_FPS_HIDE:
                fps_enabled = false;
                break;
            case CMD_OBJ_OPEN:
                if (!(tokens.size() == 2))
                {
                    comp_color = COLOR_ERROR;
                    comp_buff = "Wrong parameters. Usage: obj-open [OBJ]";
                    return;
                }

                if (loadOBJ((string("obj/") + tokens.at(1) + string(".obj")).c_str()))
                    objfile_buff = tokens.at(1);
                else
                {
                    comp_color = COLOR_ERROR;
                    comp_buff = "Object \"" + tokens.at(1) + "\" not found";
                }
                break;
            case CMD_MODE:
                if (!(tokens.size() == 2))
                {
                    comp_color = COLOR_ERROR;
                    comp_buff = "Wrong parameters. Usage: mode (face|vertex)";
                    return;
                }

                if (tokens.at(1) == "face")
                    set_mode(MODE_FACE);
                else if (tokens.at(1) == "vertex")
                    set_mode(MODE_VERTEX);
                else
                {
                    comp_color = COLOR_ERROR;
                    comp_buff = "Mode not valid. Modes available: (face|vertex)";
                    return;
                }
                break;
            case CMD_DELETE:
                if (!delete_selection())
                {
                    comp_color = COLOR_ERROR;
                    comp_buff = "Select something before trying to delete..";
                    return;
                }
                break;
            case CMD_MESS:
                mesh->mess();
                break;
            case CMD_PLANE_SHOW:
                cartesian_plane_enabled = true;
                break;
            case CMD_PLANE_HIDE:
                cartesian_plane_enabled = false;
                break;
            case CMD_PLANE_SIZE:
                if (!(tokens.size() == 2))
                {
                    comp_color = COLOR_ERROR;
                    comp_buff = "Wrong parameters. Usage: cplane-size [SIZE]";
                    return;
                }
                
                if (atoi(tokens.at(1).c_str()) <= 0)
                {
                    comp_color = COLOR_ERROR;
                    comp_buff = "Cartesian plane size must be bigger than 0";
                    return;
                }

                cartesian_plane_size = atoi(tokens.at(1).c_str());
                break;
            case CMD_RENDER_NORMAL:
                render_mode = RENDER_NORMAL;
                break;
            case CMD_RENDER_VBO:
                render_mode = RENDER_VBO;
                break;
            default:
                break;
        }

        terminal_buff.clear();
    }
}

void handleAutohelper()
{
    if (!terminal_buff.size())
        return;
    
    vector<string> tokens = StringHelper::split(terminal_buff, ' ', true);

    string buff;
    int found = 0;
    
    if (tokens.size() == 1)
    {
        for (auto cmd : terminal_cmds)
        {
            if (cmd.first.substr(0, tokens.front().size()) == tokens.front())
            {
                buff += cmd.first + " ";
                found++;
            }
        }

        if (found)
        {
            comp_color = COLOR_INFO;
            comp_buff = buff;

            if (terminal_buff.substr(0, 8) == "obj-open")
            {
                comp_color = COLOR_INFO;
                comp_buff.clear();

                for (string obj_file : obj_files)
                    comp_buff += obj_file + " ";
            }
        }
    }
    else if (tokens.size() > 1)
    {
        if (terminal_cmds[tokens.front()] == CMD_OBJ_OPEN)
        {
            for (string obj_file : obj_files)
            {
                if (obj_file.substr(0, tokens.at(1).size()) == tokens.at(1))
                {
                    buff += obj_file + " ";
                    found++;
                }
            }

            if (found)
            {
                comp_color = COLOR_INFO;
                comp_buff = buff;
            }
        }
    }
}

void handleAutocomplete()
{
    if (!terminal_buff.size())
        return;
    
    vector<string> tokens = StringHelper::split(terminal_buff, ' ', true);

    string buff;
    int found = 0;
    
    if (tokens.size() == 1)
    {
        for (auto cmd : terminal_cmds)
        {
            if (cmd.first.substr(0, tokens.front().size()) == tokens.front())
            {
                buff += cmd.first + " ";
                found++;
            }
        }

        if (found == 1)
            terminal_buff = buff;
    }
    else if (tokens.size() > 1)
    {
        if (terminal_cmds[tokens.front()] == CMD_OBJ_OPEN)
        {
            for (string obj_file : obj_files)
            {
                if (obj_file.substr(0, tokens.at(1).size()) == tokens.at(1))
                {
                    buff += obj_file + " ";
                    found++;
                }
            }

            if (found == 1)
                terminal_buff = tokens.front() + " " + buff;
        }
    }
}

bool loadOBJ(const char* s)
{
	mesh = new Mesh();
	Material::setTextCount(0);
	
    if (!Reader::readObj(s, mesh))
        return false;

    if (render_mode == RENDER_VBO)
        mesh->upload_to_gpu();
	
	int textCount = Material::getTextCount();

	ids = new unsigned int[textCount];
	glGenTextures(textCount, ids);
	int k = 0;

	for (pair<string, Material*> p : mesh->getMats())
    {
		Material* mtl = get<1>(p);
		if (mtl->hasText())
        {
			Image* img = Reader::readPpm(mtl->getTextName().c_str());
			mtl->setID(ids[k++]);
			glBindTexture(GL_TEXTURE_2D, mtl->getID());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
			// 0x80E1
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img->getWidth(), img->getHeight(), 0, 0x80E1, GL_UNSIGNED_BYTE, img->getPixels());
			free(img->getPixels());
		}
	}

    return true;
}

void switch2D(void)
{
	//Switch to 2D
	glClear(GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, width, 0, height, 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
}

void display2d(const char* msg, float x, float y, float l_size, void* font)
{
	for (unsigned int i=0;i<strlen(msg);++i)
    {
		glRasterPos2f(x, y);
		glutBitmapCharacter(font, msg[i]);
		x += l_size;
	}
}


int switchSelect(GLuint* selectBuf, int x, int y)
{
	GLint viewport[4];

	glGetIntegerv (GL_VIEWPORT, viewport);

	glSelectBuffer(BUFSIZE, selectBuf);
	glRenderMode(GL_SELECT);

	glInitNames();
	glPushName(0);

	glMatrixMode (GL_PROJECTION);
	glPushMatrix ();
	glLoadIdentity ();
	
	gluPickMatrix ((GLdouble) x, (GLdouble) (viewport[3] - y), 
				  5.0, 5.0, viewport);
	gluPerspective(45.0, width / (double)height, 0.2, 500.0);

    if (render_mode == RENDER_NORMAL)
        mesh->render(GL_SELECT, glMode);
    else if (render_mode == RENDER_VBO)
        mesh->render_gpu_data();

	glMatrixMode (GL_PROJECTION);
	glPopMatrix ();
	glFlush();

	return glRenderMode (GL_RENDER);
}

void processHits (GLint hits, GLuint buffer[])
{
   GLuint names, *ptr, minZ, *ptrNames;

   ptr = (GLuint *) buffer;
   minZ = 0xffffffff;

   for (GLint i = 0; i < hits; i++)
   {	
		names = *ptr;
		ptr++;
		if (*ptr < minZ)
        {
			minZ = *ptr;
			ptrNames = ptr + 2;
		}

		ptr += names + 2;
	}
	ptr = ptrNames;

    mesh->set_selection(ptr[0], ptr[1]);
    selection_info(MODE_FACE);
}

void toggle_terminal()
{
    terminal_mode = !terminal_mode;
    comp_buff.clear();
    terminal_buff.clear();
}

void set_mode(int mode)
{
    switch (mode)
    {
        case MODE_FACE:
            if (glMode != GL_POLYGON)
                glMode = GL_POLYGON;
            viewmode_buff = "mode face";
            break;
        case MODE_VERTEX:
            if (glMode != GL_LINE_LOOP)
                glMode = GL_LINE_LOOP;
            viewmode_buff = "mode vertex";
            break;
        default:
            break;
    }
}

bool has_face_selected()
{
    return mesh->get_selection()->face != NULL;
}

bool delete_selection()
{
    if (!has_face_selected())
        return false;

    mesh->getGroupAt(mesh->get_selection()->group_pos)->eraseFaceAt(mesh->get_selection()->face_pos);
    mesh->get_selection()->face = NULL; 
    selection_buff.clear();
    return true;
}

bool complexify()
{
    if (!has_face_selected())
        return false;

    float coords[3] = {0};
    Face* face_selected = mesh->get_selection()->face;

    for (int v : face_selected->getVerts())
    {
        for (int i = 0; i < 3; i++)
            coords[i] += mesh->getVerts()[v].getCoords()[i] / face_selected->getVerts().size();
    }

    Vertex centroid = Vertex(coords);

    mesh->addVerts(centroid);
    
    for (unsigned int i = 0; i < face_selected->getVerts().size(); i++)
    {
        Face* f = new Face();

        f->addVert(face_selected->getVerts()[i]);
        f->addVert(face_selected->getVerts()[(i+1)%(face_selected->getVerts().size())]);
        f->addVert(mesh->getVerts().size() - 1);

        mesh->getGroupAt(mesh->get_selection()->group_pos)->addFace(f);
    }

    mesh->getGroupAt(mesh->get_selection()->group_pos)->eraseFaceAt(mesh->get_selection()->face_pos);
    mesh->clear_selection();

    return true;
}

void random_complexify()
{
    int random_group_index = rand_lim(mesh->getGroups().size() - 1);
    int random_face_index = rand_lim(mesh->getGroupAt(random_group_index)->getFaces().size() - 1);

    mesh->get_selection()->group_pos = random_group_index;
    mesh->get_selection()->face_pos = random_face_index;
    mesh->get_selection()->face = mesh->getGroupAt(random_group_index)->getFaceAt(random_face_index);

    complexify();
}

void selection_info(int mode)
{
    int v = 0;

    switch (mode)
    {
        case MODE_FACE:
            v = mesh->get_selection()->face->getVerts().size();
            selection_buff = "face V: " + to_string(v);
            break;
        default:
            break;
    }
}

void draw_cartesian_plane()
{
    glLineStipple(1, 0x00FF);

    /* X positive and negative lines */
    glColor3f(1.0f, 0.0f, 0.0f);

    glDisable(GL_LINE_STIPPLE);

    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(cartesian_plane_size, 0.0f, 0.0f);
    glEnd();

    glBegin(GL_POINTS);
    glVertex3f(cartesian_plane_size, 0.0f, 0.0f);
    glEnd();

    glEnable(GL_LINE_STIPPLE);

    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(-cartesian_plane_size, 0.0f, 0.0f);
    glEnd();

    glBegin(GL_POINTS);
    glVertex3f(-cartesian_plane_size, 0.0f, 0.0f);
    glEnd();

    /* Y positive and negative lines */
    glColor3f(0.0f, 1.0f, 0.0f);

    glDisable(GL_LINE_STIPPLE);

    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, cartesian_plane_size, 0.0f);
    glEnd();

    glBegin(GL_POINTS);
    glVertex3f(0.0f, cartesian_plane_size, 0.0f);
    glEnd();

    glEnable(GL_LINE_STIPPLE);

    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, -cartesian_plane_size, 0.0f);
    glEnd();

    glBegin(GL_POINTS);
    glVertex3f(0.0f, -cartesian_plane_size, 0.0f);
    glEnd();

    /* Z positive and negative lines */
    glColor3f(0.0f, 0.0f, 1.0f);

    glDisable(GL_LINE_STIPPLE);

    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, cartesian_plane_size);
    glEnd();

    glBegin(GL_POINTS);
    glVertex3f(0.0f, 0.0f, cartesian_plane_size);
    glEnd();

    glEnable(GL_LINE_STIPPLE);

    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, -cartesian_plane_size);
    glEnd();

    glBegin(GL_POINTS);
    glVertex3f(0.0f, 0.0f, -cartesian_plane_size);
    glEnd();

    glDisable(GL_LINE_STIPPLE);
}

int rand_lim(int limit)
{
    int divisor = RAND_MAX / (limit + 1);
    int retval;

    do
        retval = rand() / divisor;
    while (retval > limit);

    return retval;
}
