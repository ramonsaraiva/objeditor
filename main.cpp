#include "camera.h"
#include "reader.h"
#include "transform.h"
#include <stdio.h> 
#include <string.h>
#include <utility>
#include <algorithm>
#include <map>
#include <time.h>

#define BUFSIZE             512

#define MODE_FACE           0
#define MODE_VERTEX         1

#define RENDER_NORMAL       0
#define RENDER_VBO          1

#define POINT_MOV           0.1 

#define CMD_EXIT            0
#define CMD_COMMANDS        6
#define CMD_DATA_SHOW       7
#define CMD_DATA_HIDE       8
#define CMD_FPS_SHOW        1
#define CMD_FPS_HIDE        2
#define CMD_OBJ_OPEN        4
#define CMD_MODE            5
#define CMD_DELETE          9
#define CMD_MESS            10
#define CMD_PLANE_SHOW      11
#define CMD_PLANE_HIDE      12
#define CMD_PLANE_SIZE      13
#define CMD_RENDER_NORMAL   14
#define CMD_RENDER_VBO      15
#define CMD_COMPLEXIFY      16
#define CMD_RAND_COMPLEXIFY 17
#define CMD_TRIANGULATE     18

using namespace std;

int width = 800;
int height = 600;

int glMode = GL_POLYGON;

map<Mesh*, Transform*> objects;
Mesh* current_mesh;
Camera* camera;

int render_mode = RENDER_NORMAL;

int currentTime, frames = 0, timebase;

char s[20];

unsigned int* ids;

float new_face_xyz[3] = {0.0};

bool terminal_mode = false;
bool fps_enabled = true;
bool data_enabled = true;
bool cartesian_plane_enabled = false;
bool new_face_mode = false;

float cartesian_plane_size = 9;

string terminal_buff;
string comp_buff;
string objfile_buff;
string viewmode_buff;
string selection_buff;
string last_terminal_buff;

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
void handle_terminal_keypress(unsigned char key);
void handle_normal_keypress(unsigned char key);
void handleMouse(int button, int state, int x, int y);
void handleSpecialKeys(int key, int x, int y);
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
bool delete_face_selected();

void selection_info();
void draw_cartesian_plane();
void draw_new_face_point();

int main(int argc, char** argv)
{
    srand(time(NULL));

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(width, height);

    glutCreateWindow("objeditor ~ marcel & ramon");
    //glutFullScreen();

    glewInit();
    init();

    glutDisplayFunc(drawScene);
    glutMouseFunc(handleMouse);
    glutReshapeFunc(handleResize);
    glutKeyboardFunc(handleKeypress);
    glutSpecialFunc(handleSpecialKeys);
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
    terminal_cmds["complexify"] = CMD_COMPLEXIFY;
    terminal_cmds["random-complexify"] = CMD_RAND_COMPLEXIFY;
    terminal_cmds["triangulate"] = CMD_TRIANGULATE;

    objfile_buff = "cube.obj";
    set_mode(MODE_FACE);

    obj_files = {"cat", "cone", "cow", "cube", "horse", "parthenon", "pyramid", "rabbit", "sphere", "teapot", "torreDiPisa", "whale"};
}

void drawScene()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    	
    for (auto object : objects)
    {
        glPushMatrix();

        float* trans = object.second->get_translate();
        float* rot = object.second->get_rotate();
        float* sca = object.second->get_scale();

        /*
        glTranslatef(trans[0], trans[1], trans[2]);
        glRotatef(90, rot[0], rot[1], rot[2]);
        glScalef(sca[0], sca[1], sca[2]);
        */

        if (render_mode == RENDER_NORMAL)
            object.first->render(GL_RENDER, glMode);
        else if (render_mode == RENDER_VBO)
            object.first->render_gpu_data();

        glPopMatrix();
    }

    if (cartesian_plane_enabled)
        draw_cartesian_plane();

    if (new_face_mode)
        draw_new_face_point();
	
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
        if (current_mesh->selection_type())
        {
            glColor3f(0.980392f, 0.501961f, 0.447059f);
            display2d(selection_buff.c_str(), 10, height - 72, 14, GLUT_BITMAP_HELVETICA_18);
        }

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
        handle_terminal_keypress(key);
    else
        handle_normal_keypress(key);

    glutPostRedisplay();
}

void handle_terminal_keypress(unsigned char key)
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

void handle_normal_keypress(unsigned char key)
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
            if (new_face_mode)
                new_face_xyz[0] -= POINT_MOV;
            else if (current_mesh->selection_type() == SELECTION_VERTEX)
                current_mesh->move_selected_vertex(MOVE_VERTEX_X_NEG);
            else
                camera->changeAngle(-0.4);
            break;
        case 'l':
        case 'L':
            if (new_face_mode)
                new_face_xyz[0] += POINT_MOV;
            else if (current_mesh->selection_type() == SELECTION_VERTEX)
                current_mesh->move_selected_vertex(MOVE_VERTEX_X_POS);
            else
                camera->changeAngle(0.4);
            break;
        case 'k':
        case 'K':
            if (new_face_mode)
                new_face_xyz[1] += POINT_MOV;
            else if (current_mesh->selection_type() == SELECTION_VERTEX)
                current_mesh->move_selected_vertex(MOVE_VERTEX_Y_POS);
            else
                camera->setDirectionY(0.01);
            break;
        case 'j':
        case 'J':
            if (new_face_mode)
                new_face_xyz[1] -= POINT_MOV;
            else if (current_mesh->selection_type() == SELECTION_VERTEX)
                current_mesh->move_selected_vertex(MOVE_VERTEX_Y_NEG);
            else
                camera->setDirectionY(-0.01);
            break;
        case 'u':
        case 'U':
            if (new_face_mode)
                new_face_xyz[2] -= POINT_MOV;
            else if (current_mesh->selection_type() == SELECTION_VERTEX)
                current_mesh->move_selected_vertex(MOVE_VERTEX_Z_NEG);
            break;
        case 'i':
        case 'I':
            if (new_face_mode)
                new_face_xyz[2] += POINT_MOV;
            else if (current_mesh->selection_type() == SELECTION_VERTEX)
                current_mesh->move_selected_vertex(MOVE_VERTEX_Z_POS);
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
            delete_face_selected();
            break;
        case 'z':
        case 'Z':
            break;
        case 'c':
        case 'C':
            if (!current_mesh->complexify())
                return;
            break;
        case 'r':
        case 'R':
            current_mesh->random_complexify();
            break;
        case 't':
        case 'T':
            current_mesh->triangulate();
            break;
        case 'n':
        case 'N':
            new_face_mode = !new_face_mode;
            break;
        case 'm':
        case 'M':
            current_mesh->mess(); 
            break;
        case 'p':
        case 'P':
            cartesian_plane_enabled = !cartesian_plane_enabled;
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
        case 13:
            if (new_face_mode)
                current_mesh->render_new_face(new_face_xyz);
            break;
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

void handleSpecialKeys(int key, int x, int y)
{
    if (!terminal_mode)
        return;

    if (key == GLUT_KEY_UP)
        terminal_buff = last_terminal_buff;
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
        last_terminal_buff = terminal_buff;

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
                if (!delete_face_selected())
                {
                    comp_color = COLOR_ERROR;
                    comp_buff = "Select something before trying to delete..";
                    return;
                }
                break;
            case CMD_MESS:
                current_mesh->mess();
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
            case CMD_COMPLEXIFY:
                if (!current_mesh->complexify())
                {
                    comp_color = COLOR_ERROR;
                    comp_buff = "Select a face before trying to complexify";
                    return;
                }
                break;
            case CMD_RAND_COMPLEXIFY:
                current_mesh->random_complexify();
                break;
            case CMD_TRIANGULATE:
                current_mesh->triangulate();
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
	Mesh* mesh = new Mesh();

    float trans[] = {0.0f, 0.0f, 0.0f};
    float rot[] = {0.0f, 0.0f, 0.0f};
    float sca[] = {1.0f, 1.0f, 1.0f};

    Transform* transform = new Transform(trans, rot, sca);

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

    objects.insert(pair<Mesh*, Transform*>(mesh, transform));
    cout << objects.size() << endl;
    current_mesh = mesh;

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
        current_mesh->render(GL_SELECT, glMode);
    else if (render_mode == RENDER_VBO)
        current_mesh->render_gpu_data();

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

    if (glMode == GL_POLYGON)
    {
        current_mesh->set_face_selected(ptr[0], ptr[1]);
        selection_info();
    }
    else if (glMode == GL_LINE_LOOP)
    {
        current_mesh->set_vertex_selected(ptr[0]);
        selection_info();
    }
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

bool delete_face_selected()
{
    if (current_mesh->selection_type() != SELECTION_FACE)
        return false;

    current_mesh->getGroupAt(current_mesh->get_face_selected()->group_pos)->eraseFaceAt(current_mesh->get_face_selected()->face_pos);
    current_mesh->get_face_selected()->face = NULL; 
    selection_buff.clear();
    return true;
}

void selection_info()
{
    int v = 0;

    switch (current_mesh->selection_type())
    {
        case SELECTION_FACE:
            v = current_mesh->get_face_selected()->face->getVerts().size();
            selection_buff = "face V: " + to_string(v);
            break;
        case SELECTION_VERTEX:
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

void draw_new_face_point()
{
    glColor3f(0.780392f, 0.0823529f, 0.521569f);

    glPushMatrix();

        glTranslatef(new_face_xyz[0], new_face_xyz[1], new_face_xyz[2]);
        glutSolidSphere(0.1, 8, 8); 

    glPopMatrix();
}
