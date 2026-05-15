#include <stdio.h>
#include <math.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include "jeu.h"
#include "anneau.h"

#define DETECTION_RAYON      (ANNEAU_RAYON_TORE + ANNEAU_RAYON_TUBE + 0.5f)
#define DETECTION_EPAISSEUR   2.5f

#define COLLISION_MARGE       1.4f
#define COLLISION_RAYON_TUBE  (ANNEAU_RAYON_TUBE + COLLISION_MARGE)
#define PROXIMITE_SEUIL      (ANNEAU_RAYON_TORE + ANNEAU_RAYON_TUBE + 3.0f)

#define SOL_Y                 0.5f   /* altitude minimale de l'avion */

void jeu_init(Jeu *j) {
    j->chrono_secondes = 0.0f;
    j->anneau_courant  = 0;
    j->chrono_actif    = 0;
    j->partie_terminee = 0;
    j->meilleur_temps  = -1.0f;
}

void jeu_reset(Jeu *j, Avion *avion) {
    j->chrono_secondes = 0.0f;
    j->anneau_courant  = 0;
    j->chrono_actif    = 0;
    j->partie_terminee = 0;
    /* meilleur_temps est conserve intentionnellement */
    avion_placer_depart(avion);
    printf("[RESET]  Retour au depart.\n");
    fflush(stdout);
}

static int collision_tube(const Avion *avion, const Anneau *a) {
    float ax = avion->trans[12];
    float ay = avion->trans[13];
    float az = avion->trans[14];

    float dx = ax - a->x;
    float dy = ay - a->y;
    float dz = az - a->z;

    float dist2_centre = dx*dx + dy*dy + dz*dz;
    if (dist2_centre > PROXIMITE_SEUIL * PROXIMITE_SEUIL)
        return 0;

    float nx = sinf(a->angle_y);
    float nz = cosf(a->angle_y);

    float dist_axiale = dx * nx + dz * nz;

    float dpx = dx - dist_axiale * nx;
    float dpy = dy;
    float dpz = dz - dist_axiale * nz;

    float dist_plan   = sqrtf(dpx*dpx + dpy*dpy + dpz*dpz);
    float dist_cercle = dist_plan - ANNEAU_RAYON_TORE;
    float dist_tube   = sqrtf(dist_cercle * dist_cercle
                             + dist_axiale  * dist_axiale);

    return dist_tube < COLLISION_RAYON_TUBE;
}

void jeu_update(Jeu *j, Avion *avion,
                const Anneau anneaux[NB_ANNEAUX], float dt)
{
    int i;

    /* --- Collision avec le sol --- */
    if (avion->trans[13] <= SOL_Y) {
        printf("[CRASH]  L'avion a touche le sol : retour au depart.\n");
        fflush(stdout);
        jeu_reset(j, avion);
        return;
    }

    /* --- Partie terminee : plus rien a faire --- */
    if (j->partie_terminee)
        return;

    /* --- Chrono (uniquement apres le 1er anneau) --- */
    if (j->chrono_actif)
        j->chrono_secondes += dt;

    /* --- Collision avec les tubes des anneaux --- */
    for (i = 0; i < NB_ANNEAUX; i++) {
        if (collision_tube(avion, &anneaux[i])) {
            printf("[COLLISION] Avion touche le tore %d : retour au depart.\n", i);
            fflush(stdout);
            jeu_reset(j, avion);
            return;
        }
    }

    if (j->anneau_courant >= NB_ANNEAUX)
        return;

    {
        const Anneau *a = &anneaux[j->anneau_courant];

        float ax = avion->trans[12];
        float ay = avion->trans[13];
        float az = avion->trans[14];

        float dx = a->x - ax;
        float dy = a->y - ay;
        float dz = a->z - az;

        float nx = sinf(a->angle_y);
        float nz = cosf(a->angle_y);

        float dist_axiale = dx * nx + dz * nz;
        if (dist_axiale < 0.0f) dist_axiale = -dist_axiale;

        if (dist_axiale > DETECTION_EPAISSEUR)
            return;

        float d2       = dx*dx + dy*dy + dz*dz;
        float axiale2  = (dx*nx + dz*nz) * (dx*nx + dz*nz);
        float radiale2 = d2 - axiale2;
        float dist_rad = (radiale2 > 0.0f) ? sqrtf(radiale2) : 0.0f;

        if (dist_rad > DETECTION_RAYON)
            return;

        /* --- Demarrage du chrono au passage du 1er anneau --- */
        if (j->anneau_courant == 0 && !j->chrono_actif) {
            j->chrono_actif    = 1;
            j->chrono_secondes = 0.0f;
            printf("[DEPART]  Chrono demarre.\n");
            fflush(stdout);
        }

        printf("[ANNEAU] Anneau %d/%d franchi ! (t=%.2fs)\n",
               j->anneau_courant + 1, NB_ANNEAUX,
               j->chrono_secondes);
        fflush(stdout);

        j->anneau_courant++;

        if (j->anneau_courant >= NB_ANNEAUX) {
            j->partie_terminee = 1;
            j->chrono_actif    = 0;
            if (j->meilleur_temps < 0.0f || j->chrono_secondes < j->meilleur_temps)
                j->meilleur_temps = j->chrono_secondes;
            printf("[FIN]    Circuit termine en %02d:%02d.%02d ! Appuyez sur R pour rejouer.\n",
                   (int)(j->chrono_secondes / 60.0f),
                   (int)(j->chrono_secondes) % 60,
                   (int)((j->chrono_secondes - (int)j->chrono_secondes) * 100.0f));
            fflush(stdout);
        }
    }
}

/* ------------------------------------------------------------------ */
/* Utilitaire : affiche une chaine en 2D                              */
/* ------------------------------------------------------------------ */

static void draw_string(const char *s, int x, int y) {
    int i;
    glRasterPos2i(x, y);
    for (i = 0; s[i] != '\0'; i++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, s[i]);
}

static int string_width(const char *s) {
    int w = 0, i;
    for (i = 0; s[i] != '\0'; i++)
        w += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, s[i]);
    return w;
}

/* ------------------------------------------------------------------ */
/* jeu_draw_hud                                                        */
/* ------------------------------------------------------------------ */

void jeu_draw_hud(const Jeu *j, int fenetre_w, int fenetre_h) {
    char texte[64];
    int  minutes, secondes, centimes, largeur_texte, pos_x;

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, fenetre_w, 0, fenetre_h, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);

    /* --- Chronometre (centre en haut) --- */
    if (j->chrono_actif || j->partie_terminee) {
        minutes  = (int)(j->chrono_secondes / 60.0f);
        secondes = (int)(j->chrono_secondes) % 60;
        centimes = (int)((j->chrono_secondes - (int)j->chrono_secondes) * 100.0f);
        sprintf(texte, "%02d:%02d.%02d", minutes, secondes, centimes);
    } else {
        sprintf(texte, "--:--.--");
    }

    largeur_texte = string_width(texte);
    pos_x = (fenetre_w - largeur_texte) / 2;

    glColor3f(0.0f, 0.0f, 0.0f);
    draw_string(texte, pos_x + 1, fenetre_h - 28);

    glColor3f(1.0f, 1.0f, 1.0f);
    draw_string(texte, pos_x, fenetre_h - 27);

    /* --- Anneau courant (centre, sous le chrono) --- */
    if (!j->partie_terminee) {
        sprintf(texte, "Anneau : %d / %d", j->anneau_courant, NB_ANNEAUX);
        largeur_texte = string_width(texte);
        pos_x = (fenetre_w - largeur_texte) / 2;

        glColor3f(0.0f, 0.0f, 0.0f);
        draw_string(texte, pos_x + 1, fenetre_h - 52);

        glColor3f(0.9f, 0.9f, 0.9f);
        draw_string(texte, pos_x, fenetre_h - 51);
    }

    /* --- Meilleur temps (haut gauche) --- */
    if (j->meilleur_temps < 0.0f) {
        sprintf(texte, "Meilleur : --:--.--");
    } else {
        minutes  = (int)(j->meilleur_temps / 60.0f);
        secondes = (int)(j->meilleur_temps) % 60;
        centimes = (int)((j->meilleur_temps - (int)j->meilleur_temps) * 100.0f);
        sprintf(texte, "Meilleur : %02d:%02d.%02d", minutes, secondes, centimes);
    }

    glColor3f(0.0f, 0.0f, 0.0f);
    draw_string(texte, 11, fenetre_h - 28);

    glColor3f(1.0f, 0.85f, 0.0f);
    draw_string(texte, 10, fenetre_h - 27);

    /* --- Ecran de fin --- */
    if (j->partie_terminee) {
        minutes  = (int)(j->chrono_secondes / 60.0f);
        secondes = (int)(j->chrono_secondes) % 60;
        centimes = (int)((j->chrono_secondes - (int)j->chrono_secondes) * 100.0f);

        sprintf(texte, "Circuit termine : %02d:%02d.%02d", minutes, secondes, centimes);
        largeur_texte = string_width(texte);
        pos_x = (fenetre_w - largeur_texte) / 2;

        glColor3f(0.0f, 0.0f, 0.0f);
        draw_string(texte, pos_x + 1, fenetre_h / 2 + 1);

        glColor3f(0.2f, 1.0f, 0.2f);
        draw_string(texte, pos_x, fenetre_h / 2);

        sprintf(texte, "Appuyez sur R pour rejouer");
        largeur_texte = string_width(texte);
        pos_x = (fenetre_w - largeur_texte) / 2;

        glColor3f(0.0f, 0.0f, 0.0f);
        draw_string(texte, pos_x + 1, fenetre_h / 2 - 29);

        glColor3f(1.0f, 1.0f, 0.2f);
        draw_string(texte, pos_x, fenetre_h / 2 - 30);
    }

    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}