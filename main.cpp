#include "camera.h"
#include "reader.h"
#include "transform.h"
#include <stdio.h> 
#include <string.h>
#include <utility>
#include <algorithm>
#include <map>
#include <iterator>
#include <time.h>

#define BUFSIZE             512

#define MODE_FACE           0
#define MODE_VERTEX         1

#define RENDER_NORMAL       0
#define RENDER_VBO          1

#define POINT_MOV           0.1 

#define LOAD_CLEAR          0
#define LOAD_ADD            1

#define SWITCH_PREV         0
#define SWITCH_NEXT         1

#define CMD_EXIT            0
#define CMD_COMMANDS        6
#define CMD_DATA_SHOW       7
#define CMD_DATA_HIDE       8
#define CMD_FPS_SHOW        1
#define CMD_FPS_HIDE        2
#define CMD_OBJ_OPEN        4
#define CMD_OBJ_ADD         20
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
#define CMD_CREATE          19
#define CMD_UNSELECT        21
#define CMD_TRANSLATE       22
#define CMD_ROTATE          23
#define CMD_SCALE           24
#define CMD_ANGLE_INC       25
#define CMD_CAMERA_SPEED    26

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
string viewmode_buff;
string selection_buff;
string last_terminal_buff;

map<string, int> terminal_cmds;
vector<string> obj_files;

static GLfloat COLOR_ERROR[] = {1.0f, 0.388235f, 0.278431f};
static GLfloat COLOR_INFO[] = {0.690196f, 0.768627f, 0.870588f};
GLfloat* comp_color = COLOR_ERROR;

float translate_buff_i[] = {0.0f, 0.0f, 0.0f};
float rotate_buff_i[] = {1.0f, 0.0f, 0.0f};
float scale_buff_i[] = {1.0f, 1.0f, 1.0f};
float ai_buff_i = 1.0;

float translate_buff[] = {0.0f, 0.0f, 0.0f};
float rotate_buff[] = {1.0f, 0.0f, 0.0f};
float scale_buff[] = {1.0f, 1.0f, 1.0f};
float ai_buff = ai_buff_i;

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
void handle_selection(GLuint* sb, int hits);
void handleTerminal();
void handleAutohelper();
void handleAutocomplete();

bool loadOBJ(const char* s, int mode);

void switch2D(void);
void display2d(const char* msg, float x, float y, float l_size, void* font);

int switchSelect(GLuint* selectBuf, int x, int y);
void processHits (GLint hits, GLuint buffer[]);

void toggle_terminal();
void set_mode(int mode);
bool has_face_selected();
bool delete_face_selected();

void switch_object(int it);

void selection_info();
void draw_cartesian_plane();
void draw_new_face_point();

bool check_parameters(int tokens_size, int needed, string usage);
void terminal_error(string error);

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

    loadOBJ("obj/cube.obj", LOAD_CLEAR);
    current_mesh->set_name("cube");

    timebase = glutGet(GLUT_ELAPSED_TIME);

    terminal_cmds["exit"] = CMD_EXIT;
    terminal_cmds["commands"] = CMD_COMMANDS;
    terminal_cmds["data-show"] = CMD_DATA_SHOW;
    terminal_cmds["data-hide"] = CMD_DATA_HIDE;
    terminal_cmds["fps-show"] = CMD_FPS_SHOW;
    terminal_cmds["fps-hide"] = CMD_FPS_HIDE;
    terminal_cmds["obj-open"] = CMD_OBJ_OPEN;
    terminal_cmds["obj-add"] = CMD_OBJ_ADD;
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
    terminal_cmds["create"] = CMD_CREATE;
    terminal_cmds["unselect"] = CMD_UNSELECT;
    terminal_cmds["translate"] = CMD_TRANSLATE;
    terminal_cmds["rotate"] = CMD_ROTATE;
    terminal_cmds["scale"] = CMD_SCALE;
    terminal_cmds["angle-inc"] = CMD_ANGLE_INC;
    terminal_cmds["camera-speed"] = CMD_CAMERA_SPEED;

    set_mode(MODE_FACE);

    obj_files = {"cat", "cone", "cow", "cube", "horse", "parthenon", "pyramid", "rabbit", "sphere", "teapot", "torreDiPisa", "whale"};
}

void drawScene()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    for (auto object : objects)
    {
        float* rot = object.second->get_rotate();
        float* trans = object.second->get_translate();
        float* sca = object.second->get_scale();

        glPushMatrix();

        if (object.second->is_rotating())
        {
            glRotatef(object.second->get_rotation_angle(), rot[0], rot[1], rot[2]);
            object.second->inc_rotation_angle();
        }
        else
            glRotatef(0, rot[0], rot[1], rot[2]);

        glTranslatef(trans[0], trans[1], trans[2]);
        glScalef(sca[0], sca[1], sca[2]);

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
        display2d(current_mesh->get_name().c_str(), 10, height - 26, 14, GLUT_BITMAP_HELVETICA_18);

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

    camera->resetView(width, height);
	glEnable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);

	
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
            camera->moveSide(1);
            break;
        case 's':
            camera->move(-1);
            break;
        case 'd':
            camera->moveSide(-1);
            break;
        case 'w':
            camera->move(1);
            break;
        case 'h':
            if (current_mesh->selection_type() == SELECTION_VERTEX)
                current_mesh->move_selected_vertex(MOVE_VERTEX_X_NEG);
            else if (new_face_mode)
                new_face_xyz[0] -= POINT_MOV;
            else
                camera->changeAngle(-0.4);
            break;
        case 'l':
            if (current_mesh->selection_type() == SELECTION_VERTEX)
                current_mesh->move_selected_vertex(MOVE_VERTEX_X_POS);
            else if (new_face_mode)
                new_face_xyz[0] += POINT_MOV;
            else
                camera->changeAngle(0.4);
            break;
        case 'k':
            if (current_mesh->selection_type() == SELECTION_VERTEX)
                current_mesh->move_selected_vertex(MOVE_VERTEX_Y_POS);
            else if (new_face_mode)
                new_face_xyz[1] += POINT_MOV;
            else
                camera->setDirectionY(0.01);
            break;
        case 'j':
            if (current_mesh->selection_type() == SELECTION_VERTEX)
                current_mesh->move_selected_vertex(MOVE_VERTEX_Y_NEG);
            else if (new_face_mode)
                new_face_xyz[1] -= POINT_MOV;
            else
                camera->setDirectionY(-0.01);
            break;
        case 'u':
            if (current_mesh->selection_type() == SELECTION_VERTEX)
                current_mesh->move_selected_vertex(MOVE_VERTEX_Z_NEG);
            else if (new_face_mode)
                new_face_xyz[2] -= POINT_MOV;
            break;
        case 'U':
            current_mesh->clear_selection();
            break;
        case 'i':
            if (current_mesh->selection_type() == SELECTION_VERTEX)
                current_mesh->move_selected_vertex(MOVE_VERTEX_Z_POS);
            else if (new_face_mode)
                new_face_xyz[2] += POINT_MOV;
            break;
        case 'f':
            set_mode(MODE_FACE);
            break;
        case 'v':
            set_mode(MODE_VERTEX);
            break;
        case 'x':
            if (current_mesh->selection_type() == SELECTION_FACE)
                current_mesh->delete_selected_face();
            break;
        case 'z':
            break;
        case 'c':
            if (!current_mesh->complexify())
                return;
            break;
        case 'r':
            current_mesh->random_complexify();
            break;
        case 't':
            current_mesh->triangulate();
            break;
        case 'n':
            new_face_mode = !new_face_mode;
            if (new_face_mode)
                current_mesh->clear_selection();
            break;
        case 'm':
            current_mesh->mess(); 
            break;
        case 'p':
            cartesian_plane_enabled = !cartesian_plane_enabled;
            break;
        case 'o':
            objects.at(current_mesh)->toggle_rotation();
            break;
        case 'q':
            exit(0);
            break;
        case 13:
            if (new_face_mode)
                current_mesh->render_new_face(new_face_xyz);
            break;
        case 32:
            GLuint sb[BUFSIZE];
            handle_selection(sb, switchSelect(sb, width/2, height/2));
            break;
        case '1':
            switch_object(SWITCH_PREV);
            break;
        case '2':
            switch_object(SWITCH_NEXT);
            break;
    }
}

void handleMouse(int button, int state, int x, int y)
{
    if (state != GLUT_DOWN)
        return;

    if (objects.at(current_mesh)->is_rotating())
        return;

    GLuint selectBuf[BUFSIZE];

	if (button == GLUT_LEFT_BUTTON)
    {
        handle_selection(selectBuf, switchSelect(selectBuf, x, y));
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

void handle_selection(GLuint* sb, GLint hits)
{
    if (hits != 0)
        processHits(hits, sb);
    else
    {
        if (current_mesh->selection_type() != SELECTION_NONE)
            current_mesh->clear_selection();
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
        terminal_error("The command \"" + tokens.front() + "\" was not found, try \"commands\" to see which is available ");
        return;
    }
    else
    {
        last_terminal_buff = terminal_buff;

        switch (terminal_cmds.at(tokens.front()))
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
                if (!check_parameters(tokens.size(), 2, "obj-open [OBJ]"))
                    return;

                for (int i = 0; i < 3; i++)
                {
                    translate_buff[i] = translate_buff_i[i];
                    rotate_buff[i] = rotate_buff_i[i];
                    scale_buff[i] = scale_buff_i[i];
                }

                ai_buff = ai_buff_i;

                if (loadOBJ((string("obj/") + tokens.at(1) + string(".obj")).c_str(), LOAD_CLEAR))
                    current_mesh->set_name(tokens.at(1));
                else
                    terminal_error("Object \"" + tokens.at(1) + "\" not found");
                break;
            case CMD_OBJ_ADD:
                if (!check_parameters(tokens.size(), 10, "obj-add [OBJ] [tX] [tY] [tZ] [rX] [rY] [rZ] [s] [rINC]"))
                    return;

                for (int i = 0; i < 3; i++)
                {
                    translate_buff[i] = atof(tokens[i+2].c_str());
                    rotate_buff[i] = atof(tokens[i+5].c_str());
                    scale_buff[i] = atof(tokens[8].c_str());
                }

                ai_buff = atof(tokens[9].c_str());

                if (loadOBJ((string("obj/") + tokens.at(1) + string(".obj")).c_str(), LOAD_ADD))
                    current_mesh->set_name(tokens.at(1));
                else
                    terminal_error("Object \"" + tokens.at(1) + "\" not found");
                break;
            case CMD_MODE:
                if (!check_parameters(tokens.size(), 2, "mode (face|vertex)"))
                    return;

                if (tokens.at(1) == "face")
                    set_mode(MODE_FACE);
                else if (tokens.at(1) == "vertex")
                    set_mode(MODE_VERTEX);
                else
                {
                    terminal_error("Mode not valid. Modes available: (face|vertex)");
                    return;
                }
                break;
            case CMD_DELETE:
                if (current_mesh->selection_type() != SELECTION_FACE)
                {
                    terminal_error("Select something before trying to delete..");
                    return;
                }

                current_mesh->delete_selected_face();
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
                if (!check_parameters(tokens.size(), 2, "cplane-size [SIZE]"))
                    return;

                if (atoi(tokens.at(1).c_str()) <= 0)
                {
                    terminal_error("Cartesian plane size must be bigger than 0");
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
                    terminal_error("Select a face before trying to complexify");
                    return;
                }
                break;
            case CMD_RAND_COMPLEXIFY:
                current_mesh->random_complexify();
                break;
            case CMD_TRIANGULATE:
                current_mesh->triangulate();
                break;
            case CMD_CREATE:
                new_face_mode = true;
                break;
            case CMD_UNSELECT:
                current_mesh->clear_selection();
                break;
            case CMD_TRANSLATE:
                if (!check_parameters(tokens.size(), 4, "translate [X] [Y] [Z]"))
                    return;

                for (int i = 0; i < 3; i++)
                    translate_buff[i] = atof(tokens[i+1].c_str());

                objects.at(current_mesh)->set_translate(translate_buff);
                break;
            case CMD_ROTATE:
                if (!check_parameters(tokens.size(), 4, "rotate [X] [Y] [Z]"))
                    return;

                for (int i = 0; i < 3; i++)
                    rotate_buff[i] = atof(tokens[i+1].c_str());

                objects.at(current_mesh)->set_rotate(rotate_buff);
                break;
            case CMD_SCALE:
                if (!check_parameters(tokens.size(), 4, "scale [X] [Y] [Z]"))
                    return;

                for (int i = 0; i < 3; i++)
                    scale_buff[i] = atof(tokens[i+1].c_str());

                objects.at(current_mesh)->set_scale(scale_buff);
                break;
            case CMD_ANGLE_INC:
                if (!check_parameters(tokens.size(), 2, "angle-inc [INC]"))
                    return;

                objects.at(current_mesh)->set_angle_inc(atof(tokens[1].c_str()));
                break;
            case CMD_CAMERA_SPEED:
                if (!check_parameters(tokens.size(), 2, "camera-speed [SPEED]"))
                    return;
                
                camera->set_speed(atof(tokens[1].c_str()));
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

            if (terminal_buff.substr(0, 8) == "obj-open" || terminal_buff.substr(0, 7) == "obj-add")
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
        if (terminal_cmds.find(tokens.front())->second == CMD_OBJ_OPEN || terminal_cmds.find(tokens.front())->second == CMD_OBJ_ADD)
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
        if (terminal_cmds.find(tokens.front())->second == CMD_OBJ_OPEN || terminal_cmds.find(tokens.front())->second == CMD_OBJ_ADD)
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

bool loadOBJ(const char* s, int mode)
{
    if (mode == LOAD_CLEAR)
    {
        objects.clear();
	    Material::setTextCount(0);
    }

	Mesh* mesh = new Mesh();

    Transform* transform = new Transform(translate_buff, rotate_buff, scale_buff, ai_buff);
	
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
    current_mesh = mesh;

    for (int i = 0; i < 3; i++)
        new_face_xyz[i] = 0;

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
				  10.0, 10.0, viewport);
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

void switch_object(int it)
{
    auto curr = objects.find(current_mesh);

    if (it == SWITCH_NEXT)
    {
        if (curr == --objects.end())
            return;

        curr = std::next(curr);
        current_mesh = curr->first;
    }
    else
    {
        if (curr == objects.begin())
            return;

        curr = std::prev(curr);
        current_mesh = curr->first;
    }


}

void selection_info()
{
    switch (current_mesh->selection_type())
    {
        case SELECTION_FACE:
            selection_buff = "face";
            break;
        case SELECTION_VERTEX:
            selection_buff = "vertex";
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

    glPushMatrix();

    if (current_mesh->selection_type() == SELECTION_VERTEX && glMode == GL_LINE_LOOP)
    {
        float* xyz = current_mesh->get_selected_vertex_xyz();
        glTranslatef(xyz[0], xyz[1], xyz[2]);
    }
    else if (new_face_mode)

    {
        glTranslatef(new_face_xyz[0], new_face_xyz[1], new_face_xyz[2]);
    }
    else
    {
        glTranslatef(0.0f, 0.0f, 0.0f);
    }

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

    glPopMatrix();
}

void draw_new_face_point()
{
    glColor3f(0.780392f, 0.0823529f, 0.521569f);

    glPushMatrix();

        glTranslatef(new_face_xyz[0], new_face_xyz[1], new_face_xyz[2]);
        glutSolidSphere(0.1, 8, 8);

    glPopMatrix();
}

bool check_parameters(int tokens_size, int needed, string usage)
{
    if (tokens_size == needed)
        return true;

    terminal_error("Wrong parameters. Usage: " + usage);
    return false;
}

void terminal_error(string error)
{
    comp_color = COLOR_ERROR;
    comp_buff = error;
}
