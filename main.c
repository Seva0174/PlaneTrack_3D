#include <stdio.h>
#include <stdlib.h>

#include <OpenGL/gl.h>
#include "GLUT/glut.h"

#include "volume.h"

int resx = 800;
int resy = 800;
int gauche, droite, haut, bas;

double angle = 0;
double vitees_rotation = 0.01;

karbre b;

void dessiner_axe()
{
    glBegin(GL_LINES);

    // X
    glColor3f(1.0, 0.0, 0.0);
    glVertex3f(-100, 0, 0);
    glVertex3f(100, 0, 0);

    // Y
    glColor3f(0.0, 1.0, 0.0);
    glVertex3f(0, -100, 0);
    glVertex3f(0, 100, 0);

    // Z
    glColor3f(0.0, 0.0, 1.0);
    glVertex3f(0, 0, -100);
    glVertex3f(0, 0, 100);

    glEnd();
}

// Exo2
void dessiner_cube()
{
    glBegin(GL_QUADS);

    // Face du bas
    glColor3f(0.9, 0.9, 0.9);
    glVertex3f(-1, -1, 1);
    glVertex3f(1, -1, 1);
    glVertex3f(1, -1, -1);
    glVertex3f(-1, -1, -1);

    // Face du haut
    glColor3f(0.8, 0.8, 0.8);
    glVertex3f(-1, 1, 1);
    glVertex3f(1, 1, 1);
    glVertex3f(1, 1, -1);
    glVertex3f(-1, 1, -1);

    // Face avant
    glColor3f(0.7, 0.7, 0.7);
    glVertex3f(-1, -1, 1);
    glVertex3f(1, -1, 1);
    glVertex3f(1, 1, 1);
    glVertex3f(-1, 1, 1);

    // Face arrière
    glColor3f(0.6, 0.6, 0.6);
    glVertex3f(-1, -1, -1);
    glVertex3f(1, -1, -1);
    glVertex3f(1, 1, -1);
    glVertex3f(-1, 1, -1);

    // Face gauche
    glColor3f(0.5, 0.5, 0.5);
    glVertex3f(-1, -1, -1);
    glVertex3f(-1, -1, 1);
    glVertex3f(-1, 1, 1);
    glVertex3f(-1, 1, -1);

    // Face droite
    glColor3f(0.4, 0.4, 0.4);
    glVertex3f(1, -1, -1);
    glVertex3f(1, -1, 1);
    glVertex3f(1, 1, 1);
    glVertex3f(1, 1, -1);

    glEnd();
}

void affichage()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // plan de projection et plan arrière
    glFrustum(-1, 1, -1, 1, 1, 100);
    // position du spectateur et la direction vers laquel il regarde
    // ici on regarde l'origine
    gluLookAt(18 * sin(angle), 5, 18 * cos(angle), 0, 0, 0, 0, 1, 0);
    // gluLookAt(2, 4, 8, 0, 0, 0, 0, 1, 0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    dessiner_axe();
    affiche_cube(-5, -5, -5, -3, -3, -3);
    affiche_volume(b);
    glutSwapBuffers();
}

void animer()
{
    angle += vitees_rotation;

    if (angle > 360)
        angle = angle - 360;
    if (angle < 0)
        angle = angle + 360;

    glutPostRedisplay();
}

void gerer_clavier(unsigned char touche, int x, int y)
{
    if (touche == 'a')
    {
        vitees_rotation = -vitees_rotation;
    }
}

int main(int argc, char *argv[])
{

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);

    glutInitWindowSize(resx, resy);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("Une Maison");
    glEnable(GL_DEPTH_TEST);
    glutDisplayFunc(affichage);
    glutIdleFunc(animer);
    glutKeyboardFunc(gerer_clavier);

    b = boule2arbre(5, 5, 5, 5);
    glutMainLoop();

    exit(EXIT_SUCCESS);
}
