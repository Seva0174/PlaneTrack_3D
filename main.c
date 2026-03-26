#include <stdio.h>
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
#define GRILLE_TAILLE   200      /* demi-taille du sol en unites      */
#define GRILLE_PAS      10       /* espacement des lignes de la grille */

/* ------------------------------------------------------------------ */
/* Variables globales                                                  */
/* ------------------------------------------------------------------ */

static Avion avion;

/* Etat des touches maintenues enfoncees */
static int touche_haut    = 0;
static int touche_bas     = 0;
static int touche_gauche  = 0;
static int touche_droite  = 0;

/* (la gestion souris se fait par recentrage — pas de position stockee) */

/* Temps pour calculer dt */
static int temps_precedent = 0;

/* ------------------------------------------------------------------ */
/* Dessin du sol (grille de reference)                                 */
/* ------------------------------------------------------------------ */

static void dessiner_sol(void)
{
    int i;
    glColor3f(0.25f, 0.25f, 0.25f);
    glBegin(GL_LINES);
    for (i = -GRILLE_TAILLE; i <= GRILLE_TAILLE; i += GRILLE_PAS) {
        glVertex3f((float)i,            0.0f, (float)-GRILLE_TAILLE);
        glVertex3f((float)i,            0.0f, (float) GRILLE_TAILLE);
        glVertex3f((float)-GRILLE_TAILLE, 0.0f, (float)i);
        glVertex3f((float) GRILLE_TAILLE, 0.0f, (float)i);
    }
    glEnd();
}

/* ------------------------------------------------------------------ */
/* Affichage d'un texte 2D a l'ecran                                  */
/* ------------------------------------------------------------------ */

static void afficher_texte_2d(int x, int y, const char *texte)
{
    const char *c;

    /* Passage en coordonnees ecran (projection orthographique) */
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, FENETRE_W, 0, FENETRE_H, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2i(x, y);
    for (c = texte; *c != '\0'; c++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    glEnable(GL_DEPTH_TEST);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

/* ------------------------------------------------------------------ */
/* Callback : affichage                                                */
/* ------------------------------------------------------------------ */

static void affichage(void)
{
    float cam_x, cam_y, cam_z;
    char buf[128];

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /* --- Camera ---
     * On reproduit manuellement gluPerspective(60, ratio, 0.5, 2000) :
     *   fov  = 60 degres  =>  tan(fov/2) = tan(30 deg) ~ 0.57735
     *   near = 0.5,  far = 2000
     *   top  = near * tan(fov/2)
     *   right = top * ratio
     */
    {
        double fov_rad = 60.0 * 3.14159265358979 / 180.0;
        double t = 0.5 * tan(fov_rad / 2.0);          /* top  = near * tan(fov/2) */
        double r = t * ((double)FENETRE_W / (double)FENETRE_H); /* right */
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glFrustum(-r, r, -t, t, 0.5, 2000.0);
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    cam_x = avion.x - avion.dir_x * CAM_DIST;
    cam_y = avion.y - avion.dir_y * CAM_DIST + CAM_HAUTEUR;
    cam_z = avion.z - avion.dir_z * CAM_DIST;

    gluLookAt(
        cam_x, cam_y, cam_z,
        avion.x, avion.y, avion.z,
        avion.up_x, avion.up_y, avion.up_z
    );

    /* --- Scene --- */
    dessiner_sol();
    avion_draw(&avion);

    /* --- HUD : position et vitesse --- */
    snprintf(buf, sizeof(buf),
             "Pos : (%.1f, %.1f, %.1f)  |  Dir : (%.2f, %.2f, %.2f)",
             avion.x, avion.y, avion.z,
             avion.dir_x, avion.dir_y, avion.dir_z);
    afficher_texte_2d(12, FENETRE_H - 26, buf);

    snprintf(buf, sizeof(buf), "Vitesse : %.0f u/s", avion.vitesse);
    afficher_texte_2d(12, FENETRE_H - 50, buf);

    afficher_texte_2d(12, 12, "ZQSD ou fleches : tangage/roulis | Souris : cap | Echap : quitter");

    glutSwapBuffers();
}

/* ------------------------------------------------------------------ */
/* Callback : timer (boucle de jeu)                                    */
/* ------------------------------------------------------------------ */

static void timer(int valeur)
{
    int temps_courant;
    float dt;

    (void)valeur;

    temps_courant = glutGet(GLUT_ELAPSED_TIME);
    dt = (temps_courant - temps_precedent) / 1000.0f;
    if (dt > 0.1f) dt = 0.1f;   /* cap pour eviter les sauts au demarrage */
    temps_precedent = temps_courant;

    /* Application des commandes clavier */
    avion.tangage = 0.0f;
    avion.roulis  = 0.0f;

    if (touche_haut)   avion.tangage =  1.0f;
    if (touche_bas)    avion.tangage = -1.0f;
    if (touche_gauche) avion.roulis  = -1.0f;
    if (touche_droite) avion.roulis  =  1.0f;

    avion_update(&avion, dt);

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
        case 27:  /* Echap */ exit(0);           break;
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

static void clavier_special_enfonce(int touche, int x, int y)
{
    (void)x; (void)y;
    switch (touche) {
        case GLUT_KEY_UP:    touche_haut    = 1; break;
        case GLUT_KEY_DOWN:  touche_bas     = 1; break;
        case GLUT_KEY_LEFT:  touche_gauche  = 1; break;
        case GLUT_KEY_RIGHT: touche_droite  = 1; break;
        default: break;
    }
}

static void clavier_special_relache(int touche, int x, int y)
{
    (void)x; (void)y;
    switch (touche) {
        case GLUT_KEY_UP:    touche_haut    = 0; break;
        case GLUT_KEY_DOWN:  touche_bas     = 0; break;
        case GLUT_KEY_LEFT:  touche_gauche  = 0; break;
        case GLUT_KEY_RIGHT: touche_droite  = 0; break;
        default: break;
    }
}

/* ------------------------------------------------------------------ */
/* Callback : mouvement souris — mode capture                         */
/*                                                                     */
/* On stocke la position precedente. Apres chaque mouvement on        */
/* recentre le curseur avec glutWarpPointer et on met a jour la       */
/* position precedente au centre. Les sauts superieurs a 200px sont   */
/* ignores : ce sont les warps de recentrage qui arrivent parfois     */
/* avec un frame de decalage selon le systeme.                        */
/* ------------------------------------------------------------------ */

#define SENSIBILITE_SOURIS  0.0003f   /* valeur basse = rotation douce */
#define SEUIL_WARP          200       /* px : saut > seuil => warp ignore */

static int souris_px = -1;   /* position precedente, -1 = non initialisee */
static int souris_py = -1;

static void souris_appliquer_yaw(float angle)
{
    float c = cosf(angle), s = sinf(angle);
    float ndx = c*avion.dir_x - s*avion.up_y*avion.dir_z + s*avion.up_z*avion.dir_y;
    float ndy = c*avion.dir_y - s*avion.up_z*avion.dir_x + s*avion.up_x*avion.dir_z;
    float ndz = c*avion.dir_z - s*avion.up_x*avion.dir_y + s*avion.up_y*avion.dir_x;
    float len = sqrtf(ndx*ndx + ndy*ndy + ndz*ndz);
    if (len > 1e-6f) {
        avion.dir_x = ndx / len;
        avion.dir_y = ndy / len;
        avion.dir_z = ndz / len;
    }
}

static void souris_appliquer_pitch(float angle)
{
    float rx = avion.dir_y*avion.up_z - avion.dir_z*avion.up_y;
    float ry = avion.dir_z*avion.up_x - avion.dir_x*avion.up_z;
    float rz = avion.dir_x*avion.up_y - avion.dir_y*avion.up_x;
    float c = cosf(angle), s = sinf(angle);
    float len;

    float ndx = c*avion.dir_x + s*(ry*avion.dir_z - rz*avion.dir_y);
    float ndy = c*avion.dir_y + s*(rz*avion.dir_x - rx*avion.dir_z);
    float ndz = c*avion.dir_z + s*(rx*avion.dir_y - ry*avion.dir_x);
    len = sqrtf(ndx*ndx + ndy*ndy + ndz*ndz);
    if (len > 1e-6f) {
        avion.dir_x = ndx / len;
        avion.dir_y = ndy / len;
        avion.dir_z = ndz / len;
    }

    float nux = c*avion.up_x + s*(ry*avion.up_z - rz*avion.up_y);
    float nuy = c*avion.up_y + s*(rz*avion.up_x - rx*avion.up_z);
    float nuz = c*avion.up_z + s*(rx*avion.up_y - ry*avion.up_x);
    len = sqrtf(nux*nux + nuy*nuy + nuz*nuz);
    if (len > 1e-6f) {
        avion.up_x = nux / len;
        avion.up_y = nuy / len;
        avion.up_z = nuz / len;
    }
}

static void souris_mouvement(int x, int y)
{
    int dx, dy;
    int cx = FENETRE_W / 2;
    int cy = FENETRE_H / 2;

    /* Premiere fois : on initialise sans appliquer de rotation */
    if (souris_px < 0) {
        souris_px = x;
        souris_py = y;
        glutWarpPointer(cx, cy);
        souris_px = cx;
        souris_py = cy;
        return;
    }

    dx = x - souris_px;
    dy = y - souris_py;

    /* Saut trop grand = warp de recentrage arrive en retard, on l'ignore */
    if (dx > SEUIL_WARP || dx < -SEUIL_WARP ||
        dy > SEUIL_WARP || dy < -SEUIL_WARP) {
        souris_px = x;
        souris_py = y;
        return;
    }

    if (dx != 0)
        souris_appliquer_yaw((float)dx * SENSIBILITE_SOURIS);

    if (dy != 0)
        souris_appliquer_pitch((float)dy * SENSIBILITE_SOURIS);

    /* Recentrage du curseur */
    glutWarpPointer(cx, cy);
    souris_px = cx;
    souris_py = cy;
}

/* ------------------------------------------------------------------ */
/* Callback : redimensionnement                                        */
/* ------------------------------------------------------------------ */

static void redimensionner(int w, int h)
{
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
}

/* ------------------------------------------------------------------ */
/* main                                                                */
/* ------------------------------------------------------------------ */

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(FENETRE_W, FENETRE_H);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Jeu de course aerienne");

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.10f, 0.13f, 0.20f, 1.0f);   /* fond bleu nuit */

    avion_init(&avion);

    glutDisplayFunc(affichage);
    glutReshapeFunc(redimensionner);
    glutKeyboardFunc(clavier_enfonce);
    glutKeyboardUpFunc(clavier_relache);
    glutSpecialFunc(clavier_special_enfonce);
    glutSpecialUpFunc(clavier_special_relache);
    glutPassiveMotionFunc(souris_mouvement);
    glutMotionFunc(souris_mouvement);

    glutSetCursor(GLUT_CURSOR_NONE);

    temps_precedent = glutGet(GLUT_ELAPSED_TIME);
    glutTimerFunc(TIMER_MS, timer, 0);

    glutMainLoop();
    return 0;
}