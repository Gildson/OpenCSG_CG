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

//variaveis de controle
static float rotacao_t = 0.0f;
static float trans_cubo = 0.0f;
static float raio_esfera = 1.2f;
static float tam_cub = 1.8f;
static float trans_c1 = 0.0f;
static float raio_c1 = 0.6f;
static float trans_c2 = 0.0f;
static float raio_c2 = 0.6f;
static float trans_c3 = 0.0f;
static float raio_c3 = 0.6f;

enum { 
    CSG_BASIC, CSG_WIDGET, CSG_GRID2D, CSG_GRID3D, CSG_CUBERACK, CSG_CONCAVE,

    ALGO_AUTOMATIC, GF_STANDARD, GF_DC, GF_OQ, SCS_STANDARD, SCS_DC, SCS_OQ,

    OFFSCREEN_AUTOMATIC, OFFSCREEN_FBO, OFFSCREEN_PBUFFER
};

std::vector<OpenCSG::Primitive*> primitives;

bool               spin = true;
float              rot = 0.0f;
std::ostringstream fpsStream;

//limpeza das primitivas
void clearPrimitives() {
    for (std::vector<OpenCSG::Primitive*>::const_iterator i = primitives.begin(); i != primitives.end(); ++i) {
        OpenCSG::DisplayListPrimitive* p = 
            static_cast<OpenCSG::DisplayListPrimitive*>(*i);
        glDeleteLists(1, p->getDisplayListId());
        delete p;
    }
    
    primitives.clear();
}

//deixar
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

void init()
{
    // cor de fundo
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glShadeModel (GL_SMOOTH);
	
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
    glShadeModel (GL_SMOOTH);
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    clearPrimitives();
    
    GLuint id1 = glGenLists(1);
    glNewList(id1, GL_COMPILE);
    glPushMatrix();
    glutSolidSphere(raio_esfera, 20, 20);
    glPopMatrix();
    glEndList();
    
    GLuint id2 = glGenLists(1);
    glNewList(id2, GL_COMPILE);
    glPushMatrix();
    glTranslatef(trans_cubo, 0.0f, 0.0f);
    glutSolidCube(tam_cub);
    glPopMatrix();
    glEndList();
        
    GLuint id3 = glGenLists(1);
    glNewList(id3, GL_COMPILE);
    glPushMatrix();
    glTranslatef(trans_c1, 0.0f, -1.25f);
    solidCylinder(raio_c1, 2.5, 20, 20);
    glPopMatrix();
    glEndList();

    GLuint id4 = glGenLists(1);
    glNewList(id4, GL_COMPILE);
    glPushMatrix();
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    glTranslatef(trans_c2, 0.0f, -1.25f);
    solidCylinder(raio_c2, 2.5, 20, 20);
    glPopMatrix();
    glEndList();
    
    GLuint id5 = glGenLists(1);
    glNewList(id5, GL_COMPILE);
    glPushMatrix();
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
    glTranslatef(trans_c3, 0.0f, -1.25f);
    solidCylinder(raio_c3, 2.5, 20, 20);
    glPopMatrix();
    glEndList();
 
    primitives.push_back(new OpenCSG::DisplayListPrimitive(id1, OpenCSG::Intersection, 1));
    primitives.push_back(new OpenCSG::DisplayListPrimitive(id2, OpenCSG::Intersection, 1));
    primitives.push_back(new OpenCSG::DisplayListPrimitive(id3, OpenCSG::Subtraction, 1));
    primitives.push_back(new OpenCSG::DisplayListPrimitive(id4, OpenCSG::Subtraction, 1));
    primitives.push_back(new OpenCSG::DisplayListPrimitive(id5, OpenCSG::Subtraction, 1));
    
    //função de renderização
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
   gluPerspective(60.0, (GLsizei) w/(GLsizei) h, 0.2, 100.0);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   gluLookAt(5.0, 5.0, 10.0, 0, 0, 0, 0, 1, 0);
}

void keyboard (unsigned char key, int x, int y){
	switch (key) {
		//Aumenta raio da esfera
		case 'q':
			raio_esfera = (raio_esfera + 0.1);
			glutPostRedisplay();
		break;
		
		//Diminui raio da esfera		
		case 'w':
			raio_esfera = (raio_esfera - 0.1);
			glutPostRedisplay();
		break;
		
		//Aumenta o tamanho do cubo
		case 'e':
			tam_cub = (tam_cub + 0.1);
			glutPostRedisplay();
		break;
		
		//Diminui o tamanho do cubo
		case 'r':
			tam_cub = (tam_cub - 0.1);
			glutPostRedisplay();
		break;

		//Afasta o cubo para direita
		case 't':
			trans_cubo = (trans_cubo + 0.1);
			glutPostRedisplay();
		break;

		//Afasta o cubo para esquerda
		case 'y':
			trans_cubo = (trans_cubo - 0.1);
			glutPostRedisplay();
		break;

		//Afasta o cilindro 1 para direita
		case 'u':
			trans_c1 = (trans_c1 + 0.1);
			glutPostRedisplay();
		break;

		//Afasta o cilindro 1 para esquerda
		case 'i':
			trans_c1 = (trans_c1 - 0.1);
			glutPostRedisplay();
		break;

		//Aumenta o raio do cilindro 1
		case 'o':
			raio_c1 = (raio_c1 + 0.1);
			glutPostRedisplay();
		break;

		//Diminui o raio do cilindro 1
		case 'p':
			raio_c1 = (raio_c1 - 0.1);
			glutPostRedisplay();
		break;

		//Afasta o cilindro 2 para direita
		case 'a':
			trans_c2 = (trans_c2 + 0.1);
			glutPostRedisplay();
		break;

		//Afasta o cilindro 2 para esquerda
		case 's':
			trans_c2 = (trans_c2 - 0.1);
			glutPostRedisplay();
		break;

		//Aumenta o raio do cilindro 2
		case 'd':
			raio_c2 = (raio_c2 + 0.1);
			glutPostRedisplay();
		break;

		//Diminui o raio do cilindro 2
		case 'f':
			raio_c2 = (raio_c2 - 0.1);
			glutPostRedisplay();
		break;

		//Afasta o cilindro 3 para direita
		case 'g':
			trans_c3 = (trans_c3 + 0.1);
			glutPostRedisplay();
		break;

		//Afasta o cilindro 3 para esquerda
		case 'h':
			trans_c3 = (trans_c3 - 0.1);
			glutPostRedisplay();
		break;

		//Aumenta o raio do cilindro 3
		case 'j':
			raio_c3 = (raio_c3 + 0.1);
			glutPostRedisplay();
		break;

		//Diminui o raio do cilindro 3
		case 'k':
			raio_c3 = (raio_c3 - 0.1);
			glutPostRedisplay();
		break;

		default:
         	break;
	}
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
    glutInitWindowSize(800, 800);
    glutInitWindowPosition (0, 0);
    glutCreateWindow("Trabalho de CG com OpenCSG");

    int err = glewInit();
    if (GLEW_OK != err) {
        // problem: glewInit failed, something is seriously wrong
        std::cerr << "GLEW Error: " << glewGetErrorString(err) << std::endl;
        return 1;
    }  
    
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMainLoop();

    return 0;
}

