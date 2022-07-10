// OpenCSG - library for image-based CSG rendering for OpenGL
// Copyright (C) 2002-2022, Florian Kirsch,
// Hasso-Plattner-Institute at the University of Potsdam, Germany
//
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110 - 1301 USA.

//
// main.cpp
//
// simple example program for OpenCSG using Glut
//

#include <GL/glew.h>
#include <opencsg.h>
#include "displaylistPrimitive.h"
#include <iostream>
#include <sstream>
#include <string>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

// include glut.h after stdlib.h to avoid conflict in declaration
// of exit() with Visual Studio 2010
#ifdef __APPLE__
#include <GLUT/glut.h>
#elif  _WIN32
// glut include under windows, assuming that freeglut binary has been
// directly put into the opencsg main directory (e.g., try to use the
// freeglut 3.0.0 MSVC Package that is available from
// https://www.transmissionzero.co.uk/software/freeglut-devel)
#include <../freeglut/include/GL/glut.h>
#else
#include <GL/glut.h>
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "codes/stb_image.h"

typedef struct{
	float X;
	float Y;
	float Z;
}EixoFLOAT;

typedef struct{
	int X;
	int Y;
	int Z;
}EixoINT;

typedef struct{
	float Largura;
	float Altura;
}Janela;

static int rotacao_t = 0;

int modoCamera = 0;
EixoFLOAT camera;
EixoINT cursor;
EixoINT mouse;

int xInicial;
int yInicial;
int xMove = 0;
int yMove = 0;
float anguloAtualX = M_PI_2;
float anguloAtualY = M_PI;
int distCamera = 10;
float focoCamX = 0;
float focoCamY = 0;
float focoCamZ = 0;
float offsetX = 0;
float offsetZ = 0;

void posicao_cam(){
	float theta = (xMove/500.0)*M_PI + anguloAtualX;
	float phi = (yMove/500.0)*M_PI + anguloAtualY;

	// Define a posição de câmera (x,y,z) apartir dos parâmetros theta e phi
	// OffsetX e OffsetZ deslocamento de posição de câmera quando o foco é atribuído aos planetas
	camera.X = distCamera*cos(theta)*cos(phi) + offsetX;
	camera.Y = distCamera*sin(phi);
	camera.Z = distCamera*sin(theta)*cos(phi) + offsetZ;
	
	// Define qual o valor de phi na faixa 0 até 2pi
	float faixaAngulo = ((phi/(2.0*M_PI)) - (int)(phi/(2.0*M_PI)))*2.0*M_PI;
	
	if((faixaAngulo >= M_PI_2 && faixaAngulo <= 2*M_PI_2 )||(faixaAngulo <= -M_PI_2 && faixaAngulo >= -2*M_PI_2 ))
		// Orientação do up da câmera em -y quando -90° < phi < 90°
		gluLookAt(camera.X, camera.Y, camera.Z, focoCamX, focoCamY, focoCamZ, 0, -1, 0);
	else 
		// Orientação do up da câmera em -y quando phi < -90° e phi > 90°
        	gluLookAt(camera.X, camera.Y, camera.Z, focoCamX, focoCamY, focoCamZ, 0, 1, 0);
}

enum { 
    CSG_BASIC, CSG_WIDGET, CSG_GRID2D, CSG_GRID3D, CSG_CUBERACK, CSG_CONCAVE,

    ALGO_AUTOMATIC, GF_STANDARD, GF_DC, GF_OQ, SCS_STANDARD, SCS_DC, SCS_OQ,

    OFFSCREEN_AUTOMATIC, OFFSCREEN_FBO, OFFSCREEN_PBUFFER
};

std::vector<OpenCSG::Primitive*> primitives;

bool               spin = true;
float              rot = 0.0f;
std::ostringstream fpsStream;

void clearPrimitives() {
    for (std::vector<OpenCSG::Primitive*>::const_iterator i = primitives.begin(); i != primitives.end(); ++i) {
        OpenCSG::DisplayListPrimitive* p = 
            static_cast<OpenCSG::DisplayListPrimitive*>(*i);
        glDeleteLists(1, p->getDisplayListId());
        delete p;
    }
    
    primitives.clear();
}

void solidCylinder(GLdouble radius, GLdouble height, GLint slices, GLint stacks) {

    GLUquadricObj* qobj = gluNewQuadric();

    gluCylinder(qobj, radius, radius, height, slices, stacks);
    glScalef(-1.0f, 1.0f, -1.0f);
    gluDisk(qobj, 0.0, radius, slices, stacks);
    glScalef(-1.0f, 1.0f, -1.0f);
    glTranslatef(0.0f, 0.0f, static_cast<GLfloat>(height));
    gluDisk(qobj, 0.0, radius, slices, stacks);

    gluDeleteQuadric(qobj);    
}

void setBasicShape() {

    clearPrimitives();

    GLuint id1 = glGenLists(1);
    glNewList(id1, GL_COMPILE);
    glPushMatrix();
    glTranslatef(-0.25f, 0.0f, 0.0f);
    glutSolidSphere(1.0, 20, 20);
    glPopMatrix();
    glEndList();

    GLuint id2 = glGenLists(1);
    glNewList(id2, GL_COMPILE);
    glPushMatrix();
    glTranslatef(0.25f, 0.0f, 0.0f);
    glutSolidSphere(1.0, 20, 20);
    glPopMatrix();
    glEndList();

    GLuint id3 = glGenLists(1);
    glNewList(id3, GL_COMPILE);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.5f);
    glScalef(0.5f, 0.5f, 2.0f);
    glutSolidSphere(1.0, 20, 20);
    glPopMatrix();
    glEndList();

    primitives.push_back(new OpenCSG::DisplayListPrimitive(id1, OpenCSG::Intersection, 1));
    primitives.push_back(new OpenCSG::DisplayListPrimitive(id2, OpenCSG::Intersection, 1));
    primitives.push_back(new OpenCSG::DisplayListPrimitive(id3, OpenCSG::Subtraction, 1));
}

void setWidget() {
    
    clearPrimitives();

    GLuint id1 = glGenLists(1);
    glNewList(id1, GL_COMPILE);
    glutSolidSphere(1.2, 20, 20);
    glEndList();

    GLuint id2 = glGenLists(1);
    glNewList(id2, GL_COMPILE);
    glutSolidCube(1.8);
    glEndList();

    GLuint id3 = glGenLists(1);
    glNewList(id3, GL_COMPILE);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -1.25f);
    solidCylinder(0.6, 2.5, 20, 20);
    glPopMatrix();
    glEndList();

    GLuint id4 = glGenLists(1);
    glNewList(id4, GL_COMPILE);
    glPushMatrix();
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
    glTranslatef(0.0f, 0.0f, -1.25f);
    solidCylinder(0.6, 2.5, 20, 20);
    glPopMatrix();
    glEndList();

    GLuint id5 = glGenLists(1);
    glNewList(id5, GL_COMPILE);
    glPushMatrix();
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, 0.0f, -1.25f);
    solidCylinder(0.6, 2.5, 20, 20);
    glPopMatrix();
    glEndList();

    primitives.push_back(new OpenCSG::DisplayListPrimitive(id1, OpenCSG::Intersection, 1));
    primitives.push_back(new OpenCSG::DisplayListPrimitive(id2, OpenCSG::Intersection, 1));
    primitives.push_back(new OpenCSG::DisplayListPrimitive(id3, OpenCSG::Subtraction, 1));
    primitives.push_back(new OpenCSG::DisplayListPrimitive(id4, OpenCSG::Subtraction, 1));
    primitives.push_back(new OpenCSG::DisplayListPrimitive(id5, OpenCSG::Subtraction, 1));
}

void setGrid2D() {
    
    clearPrimitives();

    GLuint id1 = glGenLists(1);
    glNewList(id1, GL_COMPILE);
    glPushMatrix();
    glScalef(1.0f, 0.2f, 1.0f);
    glTranslatef(0.0f, -1.25f, 0.0f);
    glutSolidCube(2.5);
    glPopMatrix();
    glEndList();

    primitives.push_back(new OpenCSG::DisplayListPrimitive(id1, OpenCSG::Intersection, 1));

    for (int x=-2; x<=2; ++x) {
        for (int z=-2; z<=2; ++z) {
            GLuint id = glGenLists(1);
            glNewList(id, GL_COMPILE);
            glPushMatrix();
            glTranslatef(x*0.5f, 0.0f, z*0.5f);
            glutSolidSphere(0.22, 15, 15);
            glPopMatrix();
            glEndList();

            primitives.push_back(new OpenCSG::DisplayListPrimitive(id, OpenCSG::Subtraction, 1));
        }
    }
}

void setGrid3D() {
    
    clearPrimitives();

    GLuint id1 = glGenLists(1);
    glNewList(id1, GL_COMPILE);
    glutSolidCube(2.0);
    glEndList();

    primitives.push_back(new OpenCSG::DisplayListPrimitive(id1, OpenCSG::Intersection, 1));

    for (int x=-1; x<=1; ++x) {
        for (int y=-1; y<=1; ++y) {
            for (int z=-1; z<=1; ++z) {
                GLuint id = glGenLists(1);
                glNewList(id, GL_COMPILE);
                glPushMatrix();
                glTranslatef(static_cast<GLfloat>(x), static_cast<GLfloat>(y), static_cast<GLfloat>(z));
                glutSolidSphere(0.58, 20, 20);
                glPopMatrix();
                glEndList();

                primitives.push_back(new OpenCSG::DisplayListPrimitive(id, OpenCSG::Subtraction, 1));
            }
        }
    }
}

void setCubeRack() {

    clearPrimitives();

    GLuint id1 = glGenLists(1);
    glNewList(id1, GL_COMPILE);
    glutSolidCube(2.0);
    glEndList();

    primitives.push_back(new OpenCSG::DisplayListPrimitive(id1, OpenCSG::Intersection, 1));

    // mx*x / my*y / mz*z loop all numbers in [-3, 3] in the following order:
    // 3, -3, 2, -2, 1, -1, 0. Compared to the trivial ordering, this makes
    // the CSG rendering less depending on the camera orientation.
    for (int x=3; x>=0; --x) {
        for (int y=3; y>=0; --y) {
            for (int z=3; z>=0; --z) {
                for (int mx=-1; mx<=1 && mx<=x; mx+=2) {
                    for (int my=-1; my<=1 && my<=y; my+=2) {
                        for (int mz=-1; mz<=1 && mz<=z; mz+=2) {
                            GLuint id = glGenLists(1);
                            glNewList(id, GL_COMPILE);
                            glPushMatrix();
                            glTranslatef(float(x*mx)/6.0f, float(y*my)/6.0f, float(z*mz)/6.0f);
                            glutSolidSphere(0.58, 20, 20);
                            glPopMatrix();
                            glEndList();

                            primitives.push_back(new OpenCSG::DisplayListPrimitive(id, OpenCSG::Subtraction, 1));
                        }
                    }
                }
            }
        }
    }
}

void setConcave() {
    
    clearPrimitives();

    GLuint id1 = glGenLists(1);
    glNewList(id1, GL_COMPILE);
    glutSolidTorus(0.6, 1.0, 25, 25);
    glEndList();
    primitives.push_back(new OpenCSG::DisplayListPrimitive(id1, OpenCSG::Intersection, 2));

    for (unsigned int i=0; i<4; ++i) {
        GLuint id = glGenLists(1);
        glNewList(id, GL_COMPILE);
        glPushMatrix();
        glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
        glRotatef(i*90.0f + 45.0f, 1.0f, 0.0f, 0.0f);
        glTranslatef(0.0f, 1.0f, 0.0f);
        glutSolidTorus(0.3, 0.6, 15, 15);
        glPopMatrix();
        glEndList();
        primitives.push_back(new OpenCSG::DisplayListPrimitive(id, OpenCSG::Subtraction, 2));
    }

    GLuint id3 = glGenLists(1);
    glNewList(id3, GL_COMPILE);
    glPushMatrix();
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
    glTranslatef(0.0f, 0.0f, -1.65f);
    solidCylinder(0.3, 3.3, 20, 20);
    glPopMatrix();
    glEndList();
    primitives.push_back(new OpenCSG::DisplayListPrimitive(id3, OpenCSG::Subtraction, 1));

    GLuint id4 = glGenLists(1);
    glNewList(id4, GL_COMPILE);
    glPushMatrix();
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, 0.0f, -1.65f);
    solidCylinder(0.3, 3.3, 20, 20);
    glPopMatrix();
    glEndList();
    primitives.push_back(new OpenCSG::DisplayListPrimitive(id4, OpenCSG::Subtraction, 1));
}

void display()
{
    //posicao_cam();
    //glMatrixMode(GL_MODELVIEW);
    //glLoadIdentity();
    //gluLookAt(0.0, 2.0, 5.0,  /* eye is at (0,2,5) */
    //          0.0, 0.0, 0.0,  /* center is at (0,0,0) */
    //          0.0, 1.0, 0.0); /* up is in positive Y direction */

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glRotatef(rotacao_t, 0.0f, 1.0f, 0.0f);

    OpenCSG::render(primitives);
    glDepthFunc(GL_EQUAL);
    for (std::vector<OpenCSG::Primitive*>::const_iterator i = primitives.begin(); i != primitives.end(); ++i) {
        (*i)->render();
    }
    glDepthFunc(GL_LESS);

    glutSwapBuffers();
}

void reshape (int w, int h)
{
   glViewport (0, 0, (GLsizei) w, (GLsizei) h); 
   glMatrixMode (GL_PROJECTION);
   glLoadIdentity ();
   gluPerspective(90.0, (GLsizei) w/(GLsizei) h, 1.0, 10.0);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   gluLookAt(0, 2.0, 5.0, 0, 0, 0, 0, 1, 0);
}


void keyboard (unsigned char key, int x, int y)
{
	switch (key) {
		case 'd':
		distCamera = 10;	
		focoCamX = focoCamX;
		focoCamY = focoCamY;
		focoCamZ = focoCamZ;
		rotacao_t = (rotacao_t + 1) % 360;
		glutPostRedisplay();
		break;
		case 'f':
		distCamera = 10;
		focoCamX = focoCamX;
		focoCamY = focoCamY;
		focoCamZ = focoCamZ;
		offsetZ = offsetZ;
		rotacao_t = (rotacao_t - 1) % 360;
		glutPostRedisplay();
		break;
     
		// Avalia o pressionamento do botão direito do mouse
		case 'a': 
		// Quando pressionado
		// Define y Inicial para o valor de y no momento em que foi pressionado
		yInicial = y;
		xInicial = x;
		// Muda o modo de câmera 2 (Zoom de câmera)
		distCamera += (y - yInicial);
		yInicial = y;

		// Limita a aproximação
		if(distCamera < 2){
			distCamera = 2;
		}

		// Limita o afastamento 
		if(distCamera > 10){
			distCamera = 10;
		}

		glutPostRedisplay();	

		break;
		// Quando e liberado
		case 's':			// Define o modo de câmera para "0" (Nada ocorre com a câmera)
		modoCamera = 0;
	        break;
		default:
         	break;
	}
}

void menu(int value) {
    switch (value) {
    case CSG_BASIC:      setBasicShape();    break;
    case CSG_WIDGET:     setWidget();        break;
    case CSG_GRID2D:     setGrid2D();        break;
    case CSG_GRID3D:     setGrid3D();        break;
    case CSG_CUBERACK:   setCubeRack();      break;
    case CSG_CONCAVE:    setConcave();       break;

    case ALGO_AUTOMATIC: OpenCSG::setOption(OpenCSG::AlgorithmSetting, OpenCSG::Automatic);
                         break;
    case GF_STANDARD:    OpenCSG::setOption(OpenCSG::AlgorithmSetting, OpenCSG::Goldfeather);
                         OpenCSG::setOption(OpenCSG::DepthComplexitySetting, OpenCSG::NoDepthComplexitySampling);
                         break;
    case GF_DC:          OpenCSG::setOption(OpenCSG::AlgorithmSetting, OpenCSG::Goldfeather);
                         OpenCSG::setOption(OpenCSG::DepthComplexitySetting, OpenCSG::DepthComplexitySampling);
                         break;
    case GF_OQ:          OpenCSG::setOption(OpenCSG::AlgorithmSetting, OpenCSG::Goldfeather);
                         OpenCSG::setOption(OpenCSG::DepthComplexitySetting, OpenCSG::OcclusionQuery);
                         break;
    case SCS_STANDARD:   OpenCSG::setOption(OpenCSG::AlgorithmSetting, OpenCSG::SCS);
                         OpenCSG::setOption(OpenCSG::DepthComplexitySetting, OpenCSG::NoDepthComplexitySampling);
                         break;
    case SCS_DC:         OpenCSG::setOption(OpenCSG::AlgorithmSetting, OpenCSG::SCS);
                         OpenCSG::setOption(OpenCSG::DepthComplexitySetting, OpenCSG::DepthComplexitySampling);
                         break;
    case SCS_OQ:         OpenCSG::setOption(OpenCSG::AlgorithmSetting, OpenCSG::SCS);
                         OpenCSG::setOption(OpenCSG::DepthComplexitySetting, OpenCSG::OcclusionQuery);
                         break;

    case OFFSCREEN_AUTOMATIC: OpenCSG::setOption(OpenCSG::OffscreenSetting, OpenCSG::AutomaticOffscreenType);
                              break;
    case OFFSCREEN_FBO:       OpenCSG::setOption(OpenCSG::OffscreenSetting, OpenCSG::FrameBufferObject);
                              break;
    case OFFSCREEN_PBUFFER:   OpenCSG::setOption(OpenCSG::OffscreenSetting, OpenCSG::PBuffer);
                              break;

    default: break;
    }
    display();
}

void init()
{
    // cor de fundo
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Cor do objeto
    GLfloat light_diffuse[]   = { 0.5f,  0.0f,  0.5f,  1.0f};  // Red diffuse light
    
    //Luz interna
    GLfloat light_position0[] = {-1.0f, -1.0f, -1.0f,  0.0f};  // Infinite light location
    
    //Luz externa
    GLfloat light_position1[] = { 1.0f,  1.0f,  1.0f,  0.0f};  // Infinite light location

    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
    glEnable(GL_LIGHT0);  
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT1, GL_POSITION, light_position1);
    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE);

    // Usa buffer depth para esconder as superficies eliminadas
    glEnable(GL_DEPTH_TEST);


    // Setup the view of the CSG shape
    glMatrixMode(GL_PROJECTION);
    gluPerspective(90.0, 1.0, 1.0, 10.0);
    glMatrixMode(GL_MODELVIEW);

    
    // starting CSG shape
    setWidget();
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitWindowSize(512, 512);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
    glutCreateWindow("Trabalho de CG com OpenCSG");

    int err = glewInit();
    if (GLEW_OK != err) {
        // problem: glewInit failed, something is seriously wrong
        std::cerr << "GLEW Error: " << glewGetErrorString(err) << std::endl;
        return 1;
    }  

    int menuShape     = glutCreateMenu(menu);
    glutAddMenuEntry("Simple",   CSG_BASIC);
    glutAddMenuEntry("Widget",   CSG_WIDGET);
    glutAddMenuEntry("2D-Grid",  CSG_GRID2D);
    glutAddMenuEntry("3D-Grid",  CSG_GRID3D);
    glutAddMenuEntry("Cuberack", CSG_CUBERACK);
    glutAddMenuEntry("Concave",  CSG_CONCAVE);
    
    int menuAlgorithm = glutCreateMenu(menu);
    glutAddMenuEntry("Automatic", ALGO_AUTOMATIC);
    glutAddMenuEntry("Goldfeather standard",GF_STANDARD);
    glutAddMenuEntry("Goldfeather depth complexity sampling", GF_DC);
    glutAddMenuEntry("Goldfeather occlusion query", GF_OQ);
    glutAddMenuEntry("SCS standard", SCS_STANDARD);
    glutAddMenuEntry("SCS depth complexity sampling", SCS_DC);
    glutAddMenuEntry("SCS occlusion query", SCS_OQ);

    int menuSettings = glutCreateMenu(menu);
    glutAddMenuEntry("Automatic", OFFSCREEN_AUTOMATIC);
    glutAddMenuEntry("Frame buffer object", OFFSCREEN_FBO);
    glutAddMenuEntry("PBuffer", OFFSCREEN_PBUFFER);

    glutCreateMenu(menu);
    glutAddSubMenu("CSG Shapes", menuShape);
    glutAddSubMenu("CSG Algorithms", menuAlgorithm);
    glutAddSubMenu("Settings", menuSettings);

    // connect to right mouse button
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    menu(OFFSCREEN_AUTOMATIC);

    init();
    glutMainLoop();

    return 0;
}

