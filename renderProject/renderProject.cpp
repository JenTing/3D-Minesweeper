// renderProject.cpp : Defines the entry point for the console application.
//


#pragma comment(lib, "C:\\Program Files (x86)\\Microsoft Visual Studio 10.0\\VC\\lib\\glew32.lib")
#include<GL\glew.h>
#include<GL\freeglut.h>
#include"trackball.h"
//this trackball is modified by aki which has no rotation itself

#include "tga.h"
#include"l3DBillboard.h"


//#include <windows.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <iostream>
using namespace std;


enum CUBE_STATE {DRAW, NOT_DRAW, DRAW_NUM, 
				MARK, NOT_MARK, 
				CHOOSE, NOT_CHOOSE};

enum LEVEL {EASY, NORMAL, HARD};
enum BOARDSIZE {SMALL, MIDDLE, LARGE};

#define BUFFER_SIZE 512
#define CUBE_SIZE 1.0f
#define WIRE_SIZE 1.005f
#define CUBE_DIST 1.01f  //jump two line width

//list name
GLuint DEFAULT_CUBE;
GLuint MARK_CUBE;
GLuint MARK_AND_CHOOSE_CUBE;
GLuint CHOOSE_CUBE;
GLuint texNum[7];
GLuint material[4];
GLuint markMat;
//GLuint back;


typedef struct Material{
	GLfloat position[3];
	GLfloat ambient[4];
	GLfloat diffuse[4];
	GLfloat specular[4];
	GLfloat emission[4];
	GLfloat shininess;
}materialInfo;

materialInfo light,light1, markCube, unMarkCube;

typedef struct Cube{
	CUBE_STATE draw;
	CUBE_STATE mark;
	CUBE_STATE choose;	//is select by mouse
	GLint num;			//is bomb(-1) or number of bomb with ...	
	GLuint type;
	GLint x;
	GLint y;
	GLint z;
}cubeInfo;

static LEVEL gameLevel = EASY;
static BOARDSIZE  boardSize = MIDDLE;

static GLint cubeNum = 5;
static GLfloat orignDist; 
static cubeInfo cube[1000];
static int lastChoose = -1;
static GLdouble fAspect;
static int bombNum = 0;
static GLuint cubeMat = 0;
static int countDelete = -1;

static int totalNum;	//total cube num
static int cubeNum_2;

static bool firstInitCube = true;

GLfloat bombPercent =0.0;

GLfloat chooseCubeColor[] = {1.0, 0.0, 0.0, 1.0};
GLfloat uchooseCubeColor[] = {0.0, 0.0, 0.0, 1.0};

GLfloat eye[3] = { 0.0, 0.0, 10.0 };
GLfloat at[3]  = { 0.0, 0.0,  0.0 };
GLfloat up[3]  = { 0.0, 1.0,  0.0 };


void Init();
void display();
void reshape (int, int);
void keyboard (unsigned char, int, int); 
void specialKeyboard (int, int, int);
void motion(int, int);
void mouse(int, int, int, int);
void InitNumTexture();
void InitMaterialTexture();
void InitCubeList();
void InitMaterial();
void InitCube();
void InitAllList();
void InitLight();
void DrawGame(GLenum);//GLenum control selection mode
void DrawCube(int);
void DrawNum(int);
void DrawBox(GLfloat, GLenum);
void DrawQuad(GLfloat);
void ProcessSelection(int, int, int, int);
void SetCubeMaterial(CUBE_STATE);
void SetWireColor(CUBE_STATE);
void SetCubeType(int);
void SetBomb();
void ProcessHits (GLint, GLuint *, int);
void createMenu();
void menuFunc(int);
void CountBomb(int, int);
void VisitCube(int);
void Visit0(int);
void Visit0sNeighbor(int);
bool CheckIsAllNotDraw(int);

//bool CheckIsAllNotDraw(int, int, int);
int  CountIndex(int x, int y, int z){return ((x-1)*cubeNum_2 + (y-1)*cubeNum + (z-1));}

bool lose = false;
void checkGameOver(void)
{
	if(countDelete == 0)
	{
		cout << "You win!!!!!!!!!"<<endl;
		glutIdleFunc(display);
	}
	else if(lose)
	{
		cout <<"You lose '<_'..."<<endl;
		glutIdleFunc(display);
	}
	return;
}




/*Main seting*/
int main(int argc, char* argv[])
{
	srand(time(NULL));

	firstInitCube = true;

	glutInit(&argc, argv);
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGBA |GLUT_DEPTH);
    glutInitWindowSize (600, 600); 
    glutInitWindowPosition (100, 100);
    glutCreateWindow (argv[0]);
	Init();
	glutDisplayFunc(display); 
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
	glutSpecialFunc(specialKeyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	
	createMenu();

	printf("Wellcome to 3D Minesweeper^_^\n\n");
	printf("It's rule is same as 2D Minesweeper,");
	printf("but the appear number is show the number of mines around this cube ,");
	printf("and around is mean up/down/left/right/front/back cube of this cube\n\n");
	printf("You can choose Minesweeper size, level by [middle button]\n");
	printf("You can choose cube by [left button]\n");
	printf("If you choose a cube, you can delete it by [left button]\n");
	printf(",or you can mark/unmark it by [right button]\n\n");
	printf("Have fun!\n");

//	glutPostRedisplay();
	glutMainLoop();

	return 0;
}

void createMenu(void)
{
	int menuID;
	int subLevelMenuID;
	int subSizeMenuID;
	int subMatMenuID;

	subLevelMenuID = glutCreateMenu(menuFunc);
		glutAddMenuEntry("Easy ",'E');
		glutAddMenuEntry("NORMAL ",'N');
		glutAddMenuEntry("Hard ",'H');

	subSizeMenuID = glutCreateMenu(menuFunc);
		glutAddMenuEntry("Small",'s');
		glutAddMenuEntry("Moddle",'m');
		glutAddMenuEntry("Large",'l');

	subMatMenuID = glutCreateMenu(menuFunc);
		glutAddMenuEntry("Stone",'t');
		glutAddMenuEntry("Brick",'b');
		glutAddMenuEntry("Wood" ,'w');
		glutAddMenuEntry("Grunge-Wall",'g');

	menuID = glutCreateMenu(menuFunc);
		glutAddSubMenu("Cube Material", subMatMenuID);
		glutAddMenuEntry("",  0);
		glutAddSubMenu("Level", subLevelMenuID);
		glutAddSubMenu("Size", subSizeMenuID);
		glutAddMenuEntry("",  0);
		glutAddMenuEntry("Quit",  0);
	glutAttachMenu(GLUT_MIDDLE_BUTTON);
}

void Init(void)
{
	tbInit(GLUT_LEFT_BUTTON);

	glClearColor (0.0, 0.0, 0.0, 0.0);
	glShadeModel (GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0); 
	glEnable(GL_LIGHT1); 
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH, GL_NICEST);


	InitLight();
	InitMaterial();
	InitAllList();
	InitCube();
}

void reshape (int w, int h)
{
	fAspect = (GLfloat) w/(GLfloat) h;

	tbReshape(w, h);
	glViewport (0, 0, (GLsizei) w, (GLsizei) h); 
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();

	gluPerspective(60.0, fAspect, 1.0, 30.0);

	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(eye[0],eye[1],eye[2], at[0],at[1],at[2], up[0],up[1],up[2]);
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//	DrawBack();

	DrawGame(GL_RENDER);

	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	gluLookAt(eye[0],eye[1],eye[2], 
			  at[0],at[1],at[2], 
			  up[0],up[1],up[2]);
	glutSwapBuffers();
}

void menuFunc(int value)
{
	switch(value)
	{
		case 'E':
		case 'e':
			gameLevel = EASY;
			InitCube();
			break;
		case 'N':
		case 'n':
			gameLevel = NORMAL;
			InitCube();
			break;
		case 'H':
		case 'h':
			gameLevel = HARD;
			InitCube();
			break;

		case 'S':
		case 's':
			cubeNum = 3;
			InitCube();
			break;
		case 'M':
		case 'm':
			cubeNum = 5;
			InitCube();
			break;
		case 'L':
		case 'l':
			cubeNum = 8;
			InitCube();
			break;


		case 'T':
		case 't':
			cubeMat = 0;
			break;
		case 'B':
		case 'b':
			cubeMat = 2;
			break;
		case 'W':
		case 'w':
			cubeMat = 1;
			break;
		case 'G':
		case 'g':
			cubeMat = 3;
			break;
	}
	glutPostRedisplay();
}

void keyboard (unsigned char key, int x, int y)
{
	switch( key)
	{
		case 27:
			exit(0);
			break;
		default:
			break;
	}
}

void specialKeyboard (int key, int x, int y)
{
	switch(key)
	{
		case GLUT_KEY_UP:
			eye[2]-=1.0;
			glutPostRedisplay();
			break;
		case GLUT_KEY_DOWN:
			eye[2]+=1.0;
			glutPostRedisplay();
			break;
	}
}

void mouse(int button, int state, int x, int y)
{
	if (button != GLUT_MIDDLE_BUTTON && state == GLUT_DOWN)
		ProcessSelection(button, state, x, y);
	tbMouse(button, state, x, y);
}

void motion(int x, int y)
{
	tbMotion(x, y);
}



/*initialize...*/

void InitAllList(void)
{
//	InitBack();
	InitMaterialTexture();
	InitNumTexture();
	InitCubeList();
}

void InitMaterialTexture(void)
{
	tgaInfo *image;
	glGenTextures(4, material);
	image = tgaLoad("Data/material/stone.tga");
	glBindTexture(GL_TEXTURE_2D,material[0]);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_S,	GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_T,	GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_MAG_FILTER	,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_MIN_FILTER	,GL_LINEAR);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image->width, image->height, 
				0, GL_RGB, GL_UNSIGNED_BYTE, image->imageData);


	image = tgaLoad("Data/material/Wood.tga");
	glBindTexture(GL_TEXTURE_2D,material[1]);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_S,	GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_T,	GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_MAG_FILTER	,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_MIN_FILTER	,GL_LINEAR);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image->width, image->height, 
				0, GL_RGB, GL_UNSIGNED_BYTE, image->imageData);



	image = tgaLoad("Data/material/brick.tga");
	glBindTexture(GL_TEXTURE_2D,material[2]);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_S,	GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_T,	GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_MAG_FILTER	,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_MIN_FILTER	,GL_LINEAR);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image->width, image->height, 
				0, GL_RGB, GL_UNSIGNED_BYTE, image->imageData);


	image = tgaLoad("Data/material/grunge-wall-texture6.tga");
	glBindTexture(GL_TEXTURE_2D,material[3]);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_S,	GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_T,	GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_MAG_FILTER	,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_MIN_FILTER	,GL_LINEAR);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image->width, image->height, 
				0, GL_RGB, GL_UNSIGNED_BYTE, image->imageData);


	//glGenTextures(1, &markMat);
	//image = tgaLoad("Data/mark/star.tga");
	//glBindTexture(GL_TEXTURE_2D, markMat);
	//glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_S,	GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_T,	GL_REPEAT);

	//glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_MAG_FILTER	,GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_MIN_FILTER	,GL_LINEAR);
	//
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->width, image->height, 
	//			0, GL_RGBA, GL_UNSIGNED_BYTE, image->imageData);

}

void InitNumTexture(void)
{
	tgaInfo *image;
	glGenTextures(7, texNum);

	image = tgaLoad("Data/number/00.tga");
	glBindTexture(GL_TEXTURE_2D,texNum[0]);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_S,	GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_T,	GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_MAG_FILTER	,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_MIN_FILTER	,GL_LINEAR);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->width, image->height, 
				0, GL_RGBA, GL_UNSIGNED_BYTE, image->imageData);


	image = tgaLoad("Data/number/01.tga");
	glBindTexture(GL_TEXTURE_2D,texNum[1]);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_S,	GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_T,	GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_MAG_FILTER	,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_MIN_FILTER	,GL_LINEAR);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->width, image->height, 
				0, GL_RGBA, GL_UNSIGNED_BYTE, image->imageData);


	image = tgaLoad("Data/number/02.tga");
	glBindTexture(GL_TEXTURE_2D,texNum[2]);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_S,	GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_T,	GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_MAG_FILTER	,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_MIN_FILTER	,GL_LINEAR);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->width, image->height, 
				0, GL_RGBA, GL_UNSIGNED_BYTE, image->imageData);


	image = tgaLoad("Data/number/03.tga");
	glBindTexture(GL_TEXTURE_2D,texNum[3]);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_S,	GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_T,	GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_MAG_FILTER	,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_MIN_FILTER	,GL_LINEAR);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->width, image->height, 
				0, GL_RGBA, GL_UNSIGNED_BYTE, image->imageData);


	image = tgaLoad("Data/number/04.tga");
	glBindTexture(GL_TEXTURE_2D,texNum[4]);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_S,	GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_T,	GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_MAG_FILTER	,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_MIN_FILTER	,GL_LINEAR);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->width, image->height, 
				0, GL_RGBA, GL_UNSIGNED_BYTE, image->imageData);


	image = tgaLoad("Data/number/05.tga");
	glBindTexture(GL_TEXTURE_2D,texNum[5]);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_S,	GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_T,	GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_MAG_FILTER	,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_MIN_FILTER	,GL_LINEAR);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->width, image->height, 
				0, GL_RGBA, GL_UNSIGNED_BYTE, image->imageData);

	image = tgaLoad("Data/number/06.tga");
	glBindTexture(GL_TEXTURE_2D,texNum[6]);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_S,	GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_T,	GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_MAG_FILTER	,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_MIN_FILTER	,GL_LINEAR);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->width, image->height, 
				0, GL_RGBA, GL_UNSIGNED_BYTE, image->imageData);
}

void InitCubeList(void)
{
	DEFAULT_CUBE = glGenLists(1);
	glNewList(DEFAULT_CUBE, GL_COMPILE);
		SetCubeMaterial(NOT_MARK);
		DrawBox(CUBE_SIZE, GL_QUADS);
		glEnable(GL_COLOR_MATERIAL);
			glLineWidth(5.0f); 
			SetWireColor(NOT_CHOOSE);
			glutWireCube(WIRE_SIZE);
		glDisable(GL_COLOR_MATERIAL);
	glEndList();

	MARK_CUBE = glGenLists(1);
	glNewList(MARK_CUBE, GL_COMPILE);
		SetCubeMaterial(MARK);
		DrawBox(CUBE_SIZE, GL_QUADS);
		glEnable(GL_COLOR_MATERIAL);
			glLineWidth(5.0f); 
			SetWireColor(NOT_CHOOSE);
			glutWireCube(WIRE_SIZE);
		glDisable(GL_COLOR_MATERIAL);
	glEndList();

	MARK_AND_CHOOSE_CUBE = glGenLists(1);
	glNewList(MARK_AND_CHOOSE_CUBE, GL_COMPILE);
		SetCubeMaterial(MARK);
		DrawBox(CUBE_SIZE, GL_QUADS);
		glEnable(GL_COLOR_MATERIAL);
			glLineWidth(7.0f); 
			SetWireColor(CHOOSE);
			glutWireCube(WIRE_SIZE);
		glDisable(GL_COLOR_MATERIAL);
	glEndList();

	CHOOSE_CUBE = glGenLists(1);
	glNewList(CHOOSE_CUBE, GL_COMPILE);
		SetCubeMaterial(NOT_MARK);
		DrawBox(CUBE_SIZE, GL_QUADS);
		glEnable(GL_COLOR_MATERIAL);
			glLineWidth(7.0f);   
			SetWireColor(CHOOSE);
			glutWireCube(WIRE_SIZE);
		glDisable(GL_COLOR_MATERIAL);
	glEndList();
}

void InitCube(void)
{
	glutIdleFunc(NULL);

	totalNum = cubeNum*cubeNum*cubeNum;
	cubeNum_2 = cubeNum*cubeNum;
	lose = false;

	//add[0]=1;				add[1]=-1; 
	//add[2]=cubeNum;			add[3]=-cubeNum; 
	//add[4]=cubeNum*cubeNum; add[5]=-cubeNum*cubeNum;

	orignDist = ((GLfloat)cubeNum+1.0f)/2 * CUBE_DIST; 

	for(int i=0; i<totalNum; i++)
	{
		cube[i].choose = NOT_CHOOSE;
		cube[i].draw   = DRAW;
		cube[i].mark   = NOT_MARK;
		cube[i].type   = DEFAULT_CUBE;
		cube[i].num    = 0;
		cube[i].x      = 0;
		cube[i].y      = 0;
		cube[i].z      = 0;
	}
	SetBomb();	
}

void InitMaterial(void)
{
	//unmark cube material set
	unMarkCube.position[0] = 0;
	unMarkCube.position[1] = 0;
	unMarkCube.position[2] = 0;

	unMarkCube.ambient[0] = 1.0;
	unMarkCube.ambient[1] = 1.0;
	unMarkCube.ambient[2] = 0.0;
	unMarkCube.ambient[3] = 1.0;

	unMarkCube.diffuse[0] = 1.0;
	unMarkCube.diffuse[1] = 1.0;
	unMarkCube.diffuse[2] = 0.0;
	unMarkCube.diffuse[3] = 1.0;

	unMarkCube.emission[0] = 0.0;
	unMarkCube.emission[1] = 0.0;
	unMarkCube.emission[2] = 0.0;
	unMarkCube.emission[3] = 0.0;

	unMarkCube.specular[0] = 1.0;
	unMarkCube.specular[1] = 1.0;
	unMarkCube.specular[2] = 1.0;
	unMarkCube.specular[3] = 1.0;

	unMarkCube.shininess = 50.0;


	//mark cube material set
	markCube.position[0] = 0;
	markCube.position[1] = 0;
	markCube.position[2] = 0;

	markCube.ambient[0] = 1.0;
	markCube.ambient[1] = 0.0;
	markCube.ambient[2] = 0.0;
	markCube.ambient[3] = 1.0;

	markCube.diffuse[0] = 1.0;
	markCube.diffuse[1] = 0.0;
	markCube.diffuse[2] = 0.0;
	markCube.diffuse[3] = 1.0;

	markCube.emission[0] = 0.0;
	markCube.emission[1] = 0.0;
	markCube.emission[2] = 0.0;
	markCube.emission[3] = 0.0;

	markCube.specular[0] = 1.0;
	markCube.specular[1] = 1.0;
	markCube.specular[2] = 1.0;
	markCube.specular[3] = 1.0;

	markCube.shininess = 50.0;
}

//void InitBack(void)
//{
//	tgaInfo *image;
//	glGenTextures(1, &back);
//	image = tgaLoad("Data/starsky.tga");
//	glBindTexture(GL_TEXTURE_2D,back);
//	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_S,	GL_REPEAT);
//	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_T,	GL_REPEAT);
//
//	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_MAG_FILTER	,GL_NEAREST);
//	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_MIN_FILTER	,GL_LINEAR);
//	
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->width, image->height, 
//				0, GL_RGBA, GL_UNSIGNED_BYTE, image->imageData);
//
//}
//
//void DrawBack(void)
//{
//	glDisable(GL_LIGHTING);
//	glEnable(GL_TEXTURE_2D);	
//
//	l3dBillboardCheatSphericalBegin();
//
//	glBindTexture(GL_TEXTURE_2D,back);
//	
//	glColor4f(1.0, 1.0, 1.0, 0.9);	
//	glBegin(GL_QUADS);
//		glNormal3f(0.0, 0.0, 1.0);
//		glTexCoord2f(-2.0,-2.0);glVertex3f(-28.0f, -28.0f, -10.0);
//		glTexCoord2f(2.0,-2.0);glVertex3f( 28.0f, -28.0f, -10.0);
//		glTexCoord2f(2.0,2.0);glVertex3f( 28.0f,  28.0f, -10.0);
//		glTexCoord2f(-2.0,2.0);glVertex3f(-28.0f,  28.0f, -10.0);
//	glEnd();
//
//	glPopMatrix();
//
//	glEnable(GL_LIGHTING);
//	glDisable(GL_TEXTURE_2D);
//}

void InitLight(void)
{
	light.position[0] = 10.0;
	light.position[1] = 10.0;
	light.position[2] = 10.0;

	light.ambient[0] = 0.7;
	light.ambient[1] = 0.7;
	light.ambient[2] = 0.7;
	light.ambient[3] = 1.0;

	light.diffuse[0] = 1.0;
	light.diffuse[1] = 1.0;
	light.diffuse[2] = 1.0;
	light.diffuse[3] = 1.0;

	light.specular[0] = 1.0;
	light.specular[1] = 1.0;
	light.specular[2] = 1.0;
	light.specular[3] = 1.0;

	glLightfv(GL_LIGHT0, GL_POSITION, light.position);
	glLightfv(GL_LIGHT0, GL_AMBIENT , light.ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE , light.diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light.specular);



	light1.position[0] = -10.0;
	light1.position[1] = 10.0;
	light1.position[2] = -10.0;

	light1.ambient[0] = 0.7;
	light1.ambient[1] = 0.7;
	light1.ambient[2] = 0.7;
	light1.ambient[3] = 1.0;

	light1.diffuse[0] = 1.0;
	light1.diffuse[1] = 1.0;
	light1.diffuse[2] = 1.0;
	light1.diffuse[3] = 1.0;

	light1.specular[0] = 1.0;
	light1.specular[1] = 1.0;
	light1.specular[2] = 1.0;
	light1.specular[3] = 1.0;

	glLightfv(GL_LIGHT1, GL_POSITION, light1.position);
	glLightfv(GL_LIGHT1, GL_AMBIENT , light.ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE , light.diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light.specular);

}





/*init cube other seting*/

void SetCubeMaterial(CUBE_STATE isMark)
{
	if(isMark == MARK)
	{
		glMaterialfv(GL_FRONT, GL_AMBIENT, markCube.ambient);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, markCube.diffuse);
		glMaterialfv(GL_FRONT, GL_SPECULAR, markCube.specular);
		glMaterialfv(GL_FRONT, GL_EMISSION, markCube.emission);
		glMaterialf (GL_FRONT, GL_SHININESS, markCube.shininess);
	}
	else
	{
		glMaterialfv(GL_FRONT, GL_AMBIENT  , unMarkCube.ambient);
		glMaterialfv(GL_FRONT, GL_DIFFUSE  , unMarkCube.diffuse);
		glMaterialfv(GL_FRONT, GL_SPECULAR , unMarkCube.specular);
		glMaterialfv(GL_FRONT, GL_EMISSION , unMarkCube.emission);
		glMaterialf (GL_FRONT, GL_SHININESS, unMarkCube.shininess);
	}
}

void SetWireColor(CUBE_STATE isChoose)
{
	if(isChoose == CHOOSE)
		glColor3fv(chooseCubeColor);
	else
		glColor3fv(uchooseCubeColor);
}

void SetCubeType(int index)
{
	if(cube[index].mark == MARK)
	{
		if(cube[index].choose == CHOOSE)
			cube[index].type = MARK_AND_CHOOSE_CUBE;
		else
			cube[index].type = MARK_CUBE;
	}
	else
	{
		if(cube[index].choose == CHOOSE)	
			cube[index].type = CHOOSE_CUBE;
		else
			cube[index].type = DEFAULT_CUBE;
	}
}

void CountBomb(int index, int visit)
{	
	if(visit > totalNum || visit < -1)
		return;
	if(cube[visit].num == -1)
	{	
		cube[index].num++;
	}
	return;
}

void SetBomb(void)
{
	int tmp;

	if(gameLevel == EASY)
		bombPercent = 1/10.0f;
	else if(gameLevel == MIDDLE)
		bombPercent = 1/8.0f;
	else if(gameLevel == HARD)
		bombPercent = 1/5.0f;
	else
	{
		printf("error level number\n");
		return;
	}

	bombNum = (int)((GLfloat)totalNum*bombPercent +0.5);

	printf("\n\nThe bomb number is\t%d\n",bombNum);

	countDelete = totalNum - bombNum;

	//set bomb
	for(int i=0; i<bombNum; i++)
	{
		tmp = (int)rand()%totalNum + 1;

		while(cube[tmp].num == -1)
		{
			tmp = (int)rand()%totalNum + 1;
		}

		cube[tmp].num = -1;
		
	}

	//count 
	//coor(i,j,k) =cube[x] (x = (k-1)*cubeNum^2 + (j-1)*cubeNum^1 + (i-1)*cubeNum^0;)
	//coor(i+-1,j,k) = cube[x+-1]
	//coor(i,j+-1,k) = cube[x+-cubeNum]
	//coor(i,j,k+-1) = cube[x+-cubeNum^2]

	int neighbor;
	for(int x=1; x<=cubeNum; x++)
	{
		for(int y=1; y<=cubeNum; y++)
		{
			for(int z=1; z<=cubeNum; z++)
			{
				tmp = CountIndex(x,y,z);
				if(cube[tmp].num == -1)
					continue;

				for(int x2=(x-1); x2<=x+1; x2++)
				{
					if(x2<=0 || x2>cubeNum || x2 == x)
						continue;

					neighbor = tmp + (x2-x)*cubeNum_2;
					
					CountBomb(tmp, neighbor);
				}

				for(int y2=(y-1); y2<=y+1; y2++)
				{
					if(y2<=0 || y2>cubeNum || y2 == y)
						continue;

					neighbor = tmp +(y2-y)*cubeNum;
					CountBomb(tmp, neighbor);
				}

				for(int z2=(z-1); z2<=z+1; z2++)
				{
					if(z2<=0 || z2>cubeNum || z2 ==z)
						continue;

					neighbor = tmp +(z2-z);
					CountBomb(tmp, neighbor);
				}

			}
		}
	}
}



/*display cube*/

void DrawGame(GLenum mode)
{
	int tmp;

	glMatrixMode(GL_MODELVIEW);

	glPushMatrix();

	tbMatrix();

	if(mode == GL_SELECT)
	{
		glInitNames();		//initialize name stack
		glPushName(0);
	}
	
	//come back to origin
	glTranslatef(-orignDist, -orignDist, orignDist);

	for(int x=cubeNum; x>0 ;x--)
	{
		for(int y=cubeNum; y>0 ;y--)
		{
			for(int z=cubeNum; z>0 ;z--)
			{
				//row major...
				tmp = (x-1)*cubeNum*cubeNum + (y-1)*cubeNum + (z-1);


				if(firstInitCube){
					cube[tmp].x = x;
					cube[tmp].y = y;
					cube[tmp].z = z;
				}

				if(cube[tmp].draw == NOT_DRAW)
					continue;

				//link name and cube
				if(mode == GL_SELECT){
					glLoadName(tmp);
				}

				//come back to origin for next cube
				glTranslatef(x*CUBE_DIST, y*CUBE_DIST, -z*CUBE_DIST);
				DrawCube(tmp);
				glTranslatef(-x*CUBE_DIST, -y*CUBE_DIST, z*CUBE_DIST);

			}
		}
	}
	glPopMatrix();
	if(firstInitCube)
		firstInitCube = false;
}

void DrawCube(int index)
{

	if(cube[index].draw == DRAW_NUM)
	{
		DrawNum(cube[index].num);
	}
	else 
	{
		glEnable(GL_TEXTURE_2D);		
		glBindTexture(GL_TEXTURE_2D,material[cubeMat]);
		//if(cube[index].mark == MARK)
		//	glBindTexture(GL_TEXTURE_2D, markMat);
		glCallList(cube[index].type);
		glDisable(GL_TEXTURE_2D);
	}
}

void DrawNum(int num)
{
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH);
//	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_TEXTURE_2D);	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0);

	l3dBillboardCheatSphericalBegin();

	glBindTexture(GL_TEXTURE_2D,texNum[num]);
	
	glColor4f(1.0, 1.0, 1.0, 0.5);	
	glBegin(GL_QUADS);
		glNormal3f(0.0, 0.0, 1.0);
		glTexCoord2f(0.0,0.0);glVertex3f(-0.5f, -0.5f, 0.0);
		glTexCoord2f(1.0,0.0);glVertex3f( 0.5f, -0.5f, 0.0);
		glTexCoord2f(1.0,1.0);glVertex3f( 0.5f,  0.5f, 0.0);
		glTexCoord2f(0.0,1.0);glVertex3f(-0.5f,  0.5f, 0.0);
	glEnd();

	glPopMatrix();

	glDisable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_TEXTURE_2D);
//	glDisable(GL_COLOR_MATERIAL);
	glEnable(GL_DEPTH);
	glEnable(GL_LIGHTING);
}



/*decide the cube display way!!!!!!*/

bool CheckIsAllNotDraw(int index)
{
	int tmp;

	int x = cube[index].x;
	int y = cube[index].y;
	int z = cube[index].z;

	for(int x2=(x-1); x2<=x+1; x2++)
	{
		if(x2<=0 || x2>cubeNum || x2 == x)
			continue;
		tmp = index + (x2-x)*cubeNum_2;
		if(cube[tmp].draw == DRAW)
			return false;
	}

	for(int y2=(y-1); y2<=y+1; y2++)
	{
		if(y2<=0 || y2>cubeNum || y2 == y)
			continue;
		tmp = index + (y2-y)*cubeNum;
		if(cube[tmp].draw == DRAW)
			return false;
	}

	for(int z2=(z-1); z2<=z+1; z2++)
	{
		if(z2<=0 || z2>cubeNum || z2 ==z)
			continue;
		tmp = index + (z2-z);
		if(cube[tmp].draw == DRAW)
			return false;
	}

	return true;

}

void Visit0sNeighbor(int visit)
{
	bool testVisit;
	//case of overflow 
	if(visit >= totalNum || visit < 0)
		return;

	if(cube[visit].draw == NOT_DRAW)
		return;
	else if(cube[visit].draw ==DRAW)
	{
		if(cube[visit].num == -1)
			return;
		else if(cube[visit].num == 0)
			Visit0(visit);
		else 
		{
			testVisit = CheckIsAllNotDraw(visit);
			if(testVisit)
				cube[visit].draw = NOT_DRAW;
			else
				cube[visit].draw = DRAW_NUM;
			countDelete --;
			checkGameOver();

		}
	}
	else if(cube[visit].draw == DRAW_NUM)
	{
		//check if som draw_num is no long to draw
		testVisit  = CheckIsAllNotDraw(visit);
		if(testVisit)
			cube[visit].draw = NOT_DRAW;
	}
	else
		printf("error!!!\n");
}

void Visit0(int index)
{
	countDelete--;
	cube[index].draw = NOT_DRAW;

	int x = cube[index].x;
	int y = cube[index].y;
	int z = cube[index].z;

	int visit;

	for(int x2=(x-1); x2<=x+1; x2=x2+2)
	{
		if(x2<=0 || x2>cubeNum)
			continue;

		visit = index + (x2-x)*cubeNum_2;
		Visit0sNeighbor(visit);
	}

	for(int y2=(y-1); y2<=y+1; y2++)
	{
		if(y2<=0 || y2>cubeNum || y2 == y)
			continue;
		visit = index + (y2-y)*cubeNum;
		Visit0sNeighbor(visit);
	}

	for(int z2=(z-1); z2<=z+1; z2++)
	{
		if(z2<=0 || z2>cubeNum || z2 ==z)
			continue;
		visit = index + (z2-z);
		Visit0sNeighbor(visit);
	}

	checkGameOver();

}

void VisitCube(int index)
{
	bool testVisit;
	if(cube[index].num == -1)
	{
		lose = true;
		checkGameOver();
	}
	else if(cube[index].num == 0)
		Visit0(index);
//		Visit0Test(index);
	else
	{
		countDelete--;

		testVisit = CheckIsAllNotDraw(index);
		
		if(testVisit)
			cube[index].draw = NOT_DRAW;
		else
			cube[index].draw = DRAW_NUM;

		checkGameOver();
	}

}



/*standard pick set!!!*/
void ProcessSelection(int button, int state, int x, int y)
{
	GLuint selectBuf[BUFFER_SIZE];
	GLint hits;
	GLint viewport[4];

	glGetIntegerv (GL_VIEWPORT, viewport);
	glSelectBuffer (BUFFER_SIZE, selectBuf);

	glMatrixMode (GL_PROJECTION);
	glPushMatrix ();

	(void) glRenderMode (GL_SELECT);


	glLoadIdentity ();
/*  create 5x5 pixel picking region near cursor location	*/
	gluPickMatrix ((GLdouble) x, (GLdouble) (viewport[3] - y), 5.0, 5.0, viewport);

	//same as reshape!!!
	gluPerspective(60.0, fAspect, 1.0, 30.0);

	DrawGame(GL_SELECT);

	glMatrixMode (GL_PROJECTION);
	glPopMatrix ();
	glutSwapBuffers();
//	glFlush ();

	hits = glRenderMode (GL_RENDER);
	if(hits > 0 )
		ProcessHits (hits, selectBuf, button);

	glutPostRedisplay();
}  

void ProcessHits(GLint hits, GLuint buffer[], int button)
{
	int	coor[3] = {0};
	GLuint ii, jj, zz, names, *ptr;
	GLfloat z1;    //the z depth of object
	GLdouble min ; // find object that closer to camera
	
	ptr = (GLuint *) buffer;
	//buffer[0] : number of names conn. to object
	//buffer[1] : z depth 1 of object
	//buffer[2] : z depth 2 of object
	//buffer[3~(3+name-1)] : the name of object

	min = 99.999f;

	int tmp;

	for(int i=0; i<hits; i++){
		names = *ptr;

		z1 = (GLfloat)*(ptr+1)/0x7fffffff;
		if(z1 < min){
			min = z1;
			tmp = *(ptr+3);

		}
		ptr = ptr + 4;
	}


	if(cube[tmp].choose == CHOOSE)
	{
		if(button == GLUT_LEFT_BUTTON)
		{
			if(cube[tmp].mark == MARK)
				return;

			lastChoose = -1;
			VisitCube(tmp);
		}

		else if(button == GLUT_RIGHT_BUTTON)
		{
			if(cube[tmp].mark == MARK)
				cube[tmp].mark = NOT_MARK;
			else 
				cube[tmp].mark = MARK;
		}
	}
	else
	{
		if(button == GLUT_LEFT_BUTTON)
		{
			cube[tmp].choose = CHOOSE;
			if(lastChoose != -1)
			{
				cube[lastChoose].choose = NOT_CHOOSE;
				SetCubeType(lastChoose);
			}
			lastChoose = tmp;
		}
	}
	SetCubeType(tmp);
}




//is source of glutSolidCube
//this modified to add text coordination 
void DrawBox(GLfloat size, GLenum type)
{
	static GLfloat n[6][3] =
	{
		{-1.0, 0.0, 0.0},
		{0.0, 1.0, 0.0},
		{1.0, 0.0, 0.0},
		{0.0, -1.0, 0.0},
		{0.0, 0.0, 1.0},
		{0.0, 0.0, -1.0}
	};
	static GLint faces[6][4] =
	{
		{0, 1, 2, 3},
		{3, 2, 6, 7},
		{7, 6, 5, 4},
		{4, 5, 1, 0},
		{5, 6, 2, 1},
		{7, 4, 0, 3}
	};
	GLfloat v[8][3];
	GLint i;

	v[0][0] = v[1][0] = v[2][0] = v[3][0] = -size / 2;
	v[4][0] = v[5][0] = v[6][0] = v[7][0] = size / 2;
	v[0][1] = v[1][1] = v[4][1] = v[5][1] = -size / 2;
	v[2][1] = v[3][1] = v[6][1] = v[7][1] = size / 2;
	v[0][2] = v[3][2] = v[4][2] = v[7][2] = -size / 2;
	v[1][2] = v[2][2] = v[5][2] = v[6][2] = size / 2;

	for (i = 5; i >= 0; i--) {
		glBegin(type);
			glNormal3fv(&n[i][0]);
			glTexCoord2f(0.0,0.0);	glVertex3fv(&v[faces[i][0]][0]);
			glTexCoord2f(1.0,0.0);	glVertex3fv(&v[faces[i][1]][0]);
			glTexCoord2f(1.0,1.0);	glVertex3fv(&v[faces[i][2]][0]);
			glTexCoord2f(0.0,1.0);	glVertex3fv(&v[faces[i][3]][0]);
		glEnd();
	}
}
