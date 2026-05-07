#include <stdio.h>
#include <math.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include "jeu.h"

/* ------------------------------------------------------------------ */
/* Constantes de detection                                             */
/*                                                                     */
/* Un anneau est valide quand l'avion franchit le plan du tore et     */
/* que la distance horizontale au centre est inferieure au rayon.     */
/*                                                                     */
/* DETECTION_RAYON : rayon de la zone valide autour du centre du tore.*/
/*   On prend le rayon du tore augmente d'une marge de confort.       */
/*                                                                     */
/* DETECTION_EPAISSEUR : demi-epaisseur du plan de detection.         */
/*   Si l'avion se trouve a moins de cette distance du plan du tore   */
/*   (dans la direction perpendiculaire au portail), le passage est   */
/*   considere.                                                        */
/* ------------------------------------------------------------------ */

#define DETECTION_RAYON      (ANNEAU_RAYON_TORE + ANNEAU_RAYON_TUBE + 0.5f)
#define DETECTION_EPAISSEUR   2.5f

/* ------------------------------------------------------------------ */
/* jeu_init                                                            */
/* ------------------------------------------------------------------ */

void jeu_init(Jeu *j) {
    j->chrono_secondes = 0.0f;
    j->anneau_courant  = 0;
}

/* ------------------------------------------------------------------ */
/* jeu_update                                                          */
/*                                                                     */
/* Algorithme de detection :                                           */
/*                                                                     */
/* Le tore est oriente selon angle_y : sa normale est le vecteur      */
/*   n = ( sin(angle_y), 0, cos(angle_y) )                            */
/* (direction de vol attendue a ce portail).                           */
/*                                                                     */
/* 1. Vecteur avion -> centre de l'anneau : d = centre - avion        */
/* 2. Projection sur la normale : dist_axiale = dot(d, n)             */
/*    Si |dist_axiale| > DETECTION_EPAISSEUR, l'avion est trop loin   */
/*    du plan du portail => pas de detection.                          */
/* 3. Composante dans le plan du portail :                             */
/*    dist_radiale = sqrt(|d|^2 - dist_axiale^2)                      */
/*    Si dist_radiale > DETECTION_RAYON => en dehors du trou.          */
/* 4. Les deux conditions validees => anneau franchi.                  */
/* ------------------------------------------------------------------ */

void jeu_update(Jeu *j, const Avion *avion,
                const Anneau anneaux[NB_ANNEAUX], float dt)
{
    j->chrono_secondes += dt;

    if (j->anneau_courant >= NB_ANNEAUX)
        return;

    {
        const Anneau *a = &anneaux[j->anneau_courant];

        /* Position de l'avion */
        float ax = avion->trans[12];
        float ay = avion->trans[13];
        float az = avion->trans[14];

        /* Vecteur avion -> centre de l'anneau */
        float dx = a->x - ax;
        float dy = a->y - ay;
        float dz = a->z - az;

        /* Normale au plan du portail (direction de vol attendue) */
        float nx = sinf(a->angle_y);
        float nz = cosf(a->angle_y);

        /* Projection axiale (le long de la normale, composante Y ignoree) */
        float dist_axiale = dx * nx + dz * nz;

        if (dist_axiale < 0.0f) dist_axiale = -dist_axiale;

        if (dist_axiale > DETECTION_EPAISSEUR)
            return;

        /* Composante radiale dans le plan du portail (XZ + Y) */
        float d2        = dx*dx + dy*dy + dz*dz;
        float axiale2   = (dx*nx + dz*nz) * (dx*nx + dz*nz);
        float radiale2  = d2 - axiale2;
        float dist_rad  = (radiale2 > 0.0f) ? sqrtf(radiale2) : 0.0f;

        if (dist_rad > DETECTION_RAYON)
            return;

        /* Anneau valide */
        printf("[ANNEAU] Anneau %d/%d franchi ! (t=%.2fs)\n",
               j->anneau_courant + 1, NB_ANNEAUX,
               j->chrono_secondes);
        fflush(stdout);

        j->anneau_courant++;

        if (j->anneau_courant >= NB_ANNEAUX) {
            printf("[FIN]    Circuit termine en %02d:%02d.%02d !\n",
                   (int)(j->chrono_secondes / 60.0f),
                   (int)(j->chrono_secondes) % 60,
                   (int)((j->chrono_secondes - (int)j->chrono_secondes) * 100.0f));
            fflush(stdout);
        }
    }
}

/* ------------------------------------------------------------------ */
/* jeu_draw_hud                                                        */
/* ------------------------------------------------------------------ */

void jeu_draw_hud(const Jeu *j, int fenetre_w, int fenetre_h) {
    char texte[32];
    int  minutes;
    int  secondes;
    int  centimes;
    int  largeur_texte;
    int  pos_x;
    int  i;

    minutes  = (int)(j->chrono_secondes / 60.0f);
    secondes = (int)(j->chrono_secondes) % 60;
    centimes = (int)((j->chrono_secondes - (int)j->chrono_secondes) * 100.0f);
    sprintf(texte, "%02d:%02d.%02d", minutes, secondes, centimes);

    /* Passage en projection orthographique 2D */
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, fenetre_w, 0, fenetre_h, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);

    /* Largeur du texte pour centrage */
    largeur_texte = 0;
    for (i = 0; texte[i] != '\0'; i++)
        largeur_texte += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, texte[i]);

    pos_x = (fenetre_w - largeur_texte) / 2;

    /* Ombre portee noire */
    glColor3f(0.0f, 0.0f, 0.0f);
    glRasterPos2i(pos_x + 1, fenetre_h - 28);
    for (i = 0; texte[i] != '\0'; i++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, texte[i]);

    /* Texte blanc */
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2i(pos_x, fenetre_h - 27);
    for (i = 0; texte[i] != '\0'; i++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, texte[i]);

    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}