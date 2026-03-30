#include <stdio.h>
#include <string.h>
#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include "avion.h"

/* ------------------------------------------------------------------ */
/* Constantes                                                          */
/* ------------------------------------------------------------------ */

#define TIMER_MS        16       /* ~60 fps                           */
#define FENETRE_W       1024
#define FENETRE_H       768
#define CAM_DIST        6.0f     /* distance camera derriere l'avion  */
#define CAM_HAUTEUR     1.8f     /* offset vertical de la camera      */
#define GRILLE_TAILLE   200
#define GRILLE_PAS      10

/* ------------------------------------------------------------------ */
/* Variables globales                                                  */
/* ------------------------------------------------------------------ */

static Avion avion;

static int touche_haut    = 0;
static int touche_bas     = 0;
static int touche_gauche  = 0;
static int touche_droite  = 0;

static int mode_log       = 0;
static int temps_precedent = 0;

/* ------------------------------------------------------------------ */
/* Dessin du sol                                                       */
/* ------------------------------------------------------------------ */

static void dessiner_sol(void)
{
    int i;
    glColor3f(0.25f, 0.25f, 0.25f);
    glBegin(GL_LINES);
    for (i = -GRILLE_TAILLE; i <= GRILLE_TAILLE; i += GRILLE_PAS) {
        glVertex3f((float)i,              0.0f, (float)-GRILLE_TAILLE);
        glVertex3f((float)i,              0.0f, (float) GRILLE_TAILLE);
        glVertex3f((float)-GRILLE_TAILLE, 0.0f, (float)i);
        glVertex3f((float) GRILLE_TAILLE, 0.0f, (float)i);
    }
    glEnd();
}

/* ------------------------------------------------------------------ */
/* Callback : affichage                                                */
/* ------------------------------------------------------------------ */

static void affichage(void)
{
    float cam_x, cam_y, cam_z;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    {
        double fov_rad = 60.0 * 3.14159265358979 / 180.0;
        double t = 0.5 * tan(fov_rad / 2.0);
        double r = t * ((double)FENETRE_W / (double)FENETRE_H);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glFrustum(-r, r, -t, t, 0.5, 2000.0);
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    /* Position = colonne de translation : trans[12], trans[13], trans[14] */
    /* Direction de vol = troisieme colonne de rot : rot[8], rot[9], rot[10] */
    cam_x = avion.trans[12] - avion.rot[8]  * CAM_DIST;
    cam_y = avion.trans[13]                 + CAM_HAUTEUR;
    cam_z = avion.trans[14] - avion.rot[10] * CAM_DIST;

    gluLookAt(
        cam_x, cam_y, cam_z,
        avion.trans[12], avion.trans[13], avion.trans[14],
        0.0f, 1.0f, 0.0f          /* up monde fixe : pas de roulis */
    );

    dessiner_sol();
    avion_draw(&avion);

    glutSwapBuffers();
}

/* ------------------------------------------------------------------ */
/* Callback : timer                                                    */
/* ------------------------------------------------------------------ */

static void timer(int valeur)
{
    int temps_courant;
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

    if (mode_log) {
        /* Position dans trans[12..14], direction dans rot[8..10] */
        printf("Pos:(%.1f,%.1f,%.1f) Dir:(%.2f,%.2f,%.2f)\n",
               avion.trans[12], avion.trans[13], avion.trans[14],
               avion.rot[8], avion.rot[9], avion.rot[10]);
        fflush(stdout);
    }

    glutPostRedisplay();
    glutTimerFunc(TIMER_MS, timer, 0);
}

/* ------------------------------------------------------------------ */
/* Callbacks clavier                                                   */
/* ------------------------------------------------------------------ */

static void clavier_enfonce(unsigned char touche, int x, int y)
{
    (void)x; (void)y;
    switch (touche) {
        case 'z': case 'Z': touche_haut    = 1; break;
        case 's': case 'S': touche_bas     = 1; break;
        case 'q': case 'Q': touche_gauche  = 1; break;
        case 'd': case 'D': touche_droite  = 1; break;
        case 27:  exit(0);                       break;
        default: break;
    }
}

static void clavier_relache(unsigned char touche, int x, int y)
{
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

int main(int argc, char **argv)
{
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
    glClearColor(0.10f, 0.13f, 0.20f, 1.0f);

    avion_init(&avion);

    glutDisplayFunc(affichage);
    glutKeyboardFunc(clavier_enfonce);
    glutKeyboardUpFunc(clavier_relache);

    glutSetCursor(GLUT_CURSOR_NONE);

    if (mode_log) {
        printf("Mode log actif. Controles: Z=monter S=descendre Q=gauche D=droite Echap=quitter\n");
        fflush(stdout);
    }

    temps_precedent = glutGet(GLUT_ELAPSED_TIME);
    glutTimerFunc(TIMER_MS, timer, 0);

    glutMainLoop();
    return 0;
}