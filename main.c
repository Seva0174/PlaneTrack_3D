#include <stdio.h>
#include <string.h>
#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include "avion.h"
#include "anneau.h"
#include "jeu.h"

/* ------------------------------------------------------------------ */
/* Constantes                                                          */
/* ------------------------------------------------------------------ */

#define TIMER_MS        16
#define FENETRE_W       1024
#define FENETRE_H       768
#define CAM_DIST        6.0f
#define CAM_HAUTEUR     1.8f
#define GRILLE_TAILLE   400
#define PI              3.14159265358979

/* ------------------------------------------------------------------ */
/* Variables globales                                                  */
/* ------------------------------------------------------------------ */

Avion  avion;
Anneau anneaux[NB_ANNEAUX];
Jeu    jeu;

int touche_haut    = 0;
int touche_bas     = 0;
int touche_gauche  = 0;
int touche_droite  = 0;

int mode_log = 0;
int temps_precedent = 0;

/* ------------------------------------------------------------------ */
/* Dessin du sol                                                       */
/* ------------------------------------------------------------------ */

void dessiner_sol() {
    glColor3f(0.0f, 0.6f, 0.0f);
    glBegin(GL_QUADS);
        glVertex3f(-GRILLE_TAILLE, 0.0f, -GRILLE_TAILLE);
        glVertex3f( GRILLE_TAILLE, 0.0f, -GRILLE_TAILLE);
        glVertex3f( GRILLE_TAILLE, 0.0f,  GRILLE_TAILLE);
        glVertex3f(-GRILLE_TAILLE, 0.0f,  GRILLE_TAILLE);
    glEnd();
}

/* ------------------------------------------------------------------ */
/* Affichage                                                           */
/* ------------------------------------------------------------------ */

void affichage() {
    float cam_x, cam_y, cam_z;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    double fov_rad = 60.0 * PI / 180.0;
    double t = 0.5 * tan(fov_rad / 2.0);
    double r = t * ((double)FENETRE_W / (double)FENETRE_H);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-r, r, -t, t, 0.5, 2000.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    cam_x = avion.trans[12] - avion.rot[8]  * CAM_DIST;
    cam_y = avion.trans[13]                 + CAM_HAUTEUR;
    cam_z = avion.trans[14] - avion.rot[10] * CAM_DIST;

    gluLookAt(
        cam_x, cam_y, cam_z,
        avion.trans[12], avion.trans[13], avion.trans[14],
        0.0f, 1.0f, 0.0f
    );

    dessiner_sol();
    anneaux_draw(anneaux, jeu.anneau_courant);
    avion_draw(&avion);
    jeu_draw_hud(&jeu, FENETRE_W, FENETRE_H);

    glutSwapBuffers();
}

/* ------------------------------------------------------------------ */
/* Timer                                                               */
/* ------------------------------------------------------------------ */

void timer(int valeur) {
    int   temps_courant;
    float dt;

    (void)valeur;

    temps_courant = glutGet(GLUT_ELAPSED_TIME);
    dt = (temps_courant - temps_precedent) / 1000.0f;
    if (dt > 0.1f) dt = 0.1f;
    temps_precedent = temps_courant;

    avion.monter = 0.0f;
    avion.virer  = 0.0f;

    if (touche_haut)   avion.monter =  1.0f;
    if (touche_bas)    avion.monter = -1.0f;
    if (touche_gauche) avion.virer  = -1.0f;
    if (touche_droite) avion.virer  =  1.0f;

    avion_update(&avion, dt);
    jeu_update(&jeu, &avion, anneaux, dt);

    if (mode_log) {
        printf("Pos:(%.1f,%.1f,%.1f) Dir:(%.2f,%.2f,%.2f)\n",avion.trans[12], avion.trans[13], avion.trans[14],avion.rot[8], avion.rot[9], avion.rot[10]);
        fflush(stdout);
    }

    glutPostRedisplay();
    glutTimerFunc(TIMER_MS, timer, 0);
}

/* ------------------------------------------------------------------ */
/* Clavier                                                             */
/* ------------------------------------------------------------------ */

void clavier_enfonce(unsigned char touche, int x, int y) {
    (void)x; (void)y;
    switch (touche) {
        case 'z': case 'Z': touche_haut    = 1; break;
        case 's': case 'S': touche_bas     = 1; break;
        case 'q': case 'Q': touche_gauche  = 1; break;
        case 'd': case 'D': touche_droite  = 1; break;
        case 'r': case 'R':
            jeu_reset(&jeu, &avion);
            break;
        case 27:  exit(0); break;
        default: break;
    }
}

void clavier_relache(unsigned char touche, int x, int y) {
    (void)x; (void)y;
    switch (touche) {
        case 'z': case 'Z': touche_haut    = 0; break;
        case 's': case 'S': touche_bas     = 0; break;
        case 'q': case 'Q': touche_gauche  = 0; break;
        case 'd': case 'D': touche_droite  = 0; break;
        default: break;
    }
}

/* ------------------------------------------------------------------ */
/* main                                                                */
/* ------------------------------------------------------------------ */

int main(int argc, char **argv) {
    int i;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--log") == 0)
            mode_log = 1;
    }

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(FENETRE_W, FENETRE_H);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Jeu de course aerienne");

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.10f, 0.13f, 0.50f, 1.0f);

    avion_init(&avion);
    anneaux_init(anneaux);
    jeu_init(&jeu);

    avion_placer_depart(&avion);

    glutDisplayFunc(affichage);
    glutKeyboardFunc(clavier_enfonce);
    glutKeyboardUpFunc(clavier_relache);

    glutSetCursor(GLUT_CURSOR_NONE);

    if (mode_log) {
        printf("Mode log actif. Controles: Z=monter S=descendre Q=gauche D=droite R=rejouer Echap=quitter\n");
        fflush(stdout);
    }

    temps_precedent = glutGet(GLUT_ELAPSED_TIME);
    glutTimerFunc(TIMER_MS, timer, 0);

    glutMainLoop();
    return 0;
}